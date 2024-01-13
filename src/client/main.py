import sys
from PyQt5 import QtGui
from PyQt5.uic import loadUi
from PyQt5.QtCore import QThread, pyqtSignal, QTimer
from PyQt5.QtWidgets import QDialog, QApplication, QWidget, QStackedWidget, QButtonGroup, QMessageBox
import socket
current_players = {}
players_count = 0
my_nick = None
in_room = False
in_game = False
letter = ""
sent = False
ingameorno = False

def send_message(message):
    try:
        client_socket.send(bytes(message, "utf-8"))
    except:
        print("Zerwano połączenie z serwerem")
        sys.exit()


class ServerThread(QThread):
    new_message = pyqtSignal(str)

    def __init__(self):
        super().__init__()

    def run(self):
        while True:
            message = client_socket.recv(1024).decode("utf-8").strip().split("\n")
            if len(message) >= 1:
                for mess in message:
                    self.new_message.emit(mess.strip())


class NickScreen(QDialog):
    def __init__(self):
        super(NickScreen, self).__init__()
        loadUi("nick.ui", self)
        self.nickPushButton.clicked.connect(self.set_nick)

    def set_nick(self):
        global my_nick
        my_nick = self.nickTextEdit.toPlainText()
        if not my_nick:
            self.nickErrorLabel.setText(f"Nick nie może być pusty!")
        else:
            if len(my_nick) > 10:
                self.nickErrorLabel.setText(f"Nick nie może być dłuższy niż 10 znaków!")
            else:
                message = "SETNICK " + my_nick.strip()
                send_message(message+"\n\0")
                try:
                    status = client_socket.recv(1024).decode("utf-8").strip()
                    if len(status) > 0:
                        if status == "NICKTAKEN":
                            self.nickErrorLabel.setText(f"Nick: [{my_nick}] jest już zajęty!")
                        elif status == "NOSPACE":
                            self.nickErrorLabel.setText(f"W nicku nie moga byc spacje!")
                        else:
                            rooms = RoomsScreen()
                            widget.addWidget(rooms)
                            widget.setCurrentIndex(widget.currentIndex() + 1)
                    else:
                        print("Rozłączono z serwerem!")
                        client_socket.close()
                        sys.exit()
                except:
                    print("Błąd przy otrzymywaniu odpowiedzi od serwera")
                    client_socket.close()
                    sys.exit()


class RoomsScreen(QDialog):
    def __init__(self):
        super(RoomsScreen, self).__init__()
        loadUi("room.ui", self)
        self.update_combobox()
        self.roomSubmitButton.clicked.connect(self.join_room)
        self.roomAddButton.clicked.connect(self.add_room)
        self.refreshButton.clicked.connect(self.refresh)

    def add_room(self):
        nick = my_nick.strip()
        send_message("CREATEROOM "+ nick + " 9"+"\n\0")
        self.update_combobox()

    def refresh(self):
        self.update_combobox()

    def update_combobox(self):
        self.roomComboBox.clear()
        send_message("ROOMS\n\0")
        rooms = client_socket.recv(1024).decode("utf-8").strip().split()[1:]
        for room in rooms:
            self.roomComboBox.addItem(room[5:])

    def join_room(self):
        global in_room
        global players_count
        global current_players
        global ingameorno
        chosen_room = self.roomComboBox.currentText().split("|")[0]
        send_message("JOIN " + chosen_room+"\n\0")
        status = client_socket.recv(1024).decode("utf-8").strip().split()
        for stat in status:
            if stat == "ROOMFULL":
                msg = QMessageBox()
                msg.setWindowTitle("Błąd!")
                msg.setText("Pokój jest pełny")
                msg.exec_()
                self.update_combobox()
                break
            if stat == "JOINED|" + my_nick:
                in_room = True
                current_players[my_nick] = [0, 0]

            if stat == "INGAME":
                ingameorno = True

            if stat.startswith("CURRENTPLAYERS"):
                for player in stat.split("|")[1:-1]:
                    players_count += 1
                    current_players[player] = [players_count, 0]

                if in_room:
                    game = GameScreen()
                    widget.addWidget(game)
                    widget.setCurrentIndex(widget.currentIndex() + 1)


class GameScreen(QDialog):
    def __init__(self):
        super(GameScreen, self).__init__()
        loadUi("game.ui", self)
        global ingameorno
        self.server_thread = ServerThread()
        self.server_thread.new_message.connect(self.update_screen)
        self.server_thread.start()
        self.display_start()
        self.gameScores = [
            self.gameLabelMyScore,
            self.gameLabelPlayer1,
            self.gameLabelPlayer2,
            self.gameLabelPlayer3,
            self.gameLabelPlayer4,
            self.gameLabelPlayer5,
            self.gameLabelPlayer6,
            self.gameLabelPlayer7,
            self.gameLabelPlayer8,
                ]

        self.gameCheck = [
            self.labelCheckMyScore,
            self.labelCheck1,
            self.labelCheck2,
            self.labelCheck3,
            self.labelCheck4,
            self.labelCheck5,
            self.labelCheck6,
            self.labelCheck7,
            self.labelCheck8,
                ]

        self.setup_current_players()
        self.gameButtonStart.clicked.connect(self.start_game)
        self.generatedLetter = self.letterLabel
        self.sendButton = self.gameButtonSend
        self.labelCheckMyScore.setVisible(False)
        self.labelCheck1.setVisible(False)
        self.labelCheck2.setVisible(False)
        self.labelCheck3.setVisible(False)
        self.labelCheck4.setVisible(False)
        self.labelCheck5.setVisible(False)
        self.labelCheck6.setVisible(False)
        self.labelCheck7.setVisible(False)
        self.labelCheck8.setVisible(False)
        self.sendButton.setVisible(False)
        self.sendButton.clicked.connect(self.send_answers)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_timer)
        self.time_remaining = 60  # Ustaw czas początkowy


        self.timerLabel = self.gameTimerLabel
        self.timerLabel.setText(f"Time: {self.time_remaining} s")

        print(ingameorno)
        if ingameorno == True:
            self.gameButtonStart.setVisible(False)
            self.send_answers()




    def update_screen(self, message):
        global letter
        global players_count


        if message.startswith("JOINED"):
            players_count += 1
            player_nick = message.split("|")[1]
            current_players[player_nick] = [players_count, 0]
            self.gameScores[players_count].setText(f"{player_nick}: 0")
            self.gameScores[players_count].setVisible(True)


        elif message.startswith("CURRENTPLAYERS"):
            for player in message.split("|")[1:-1]:
                players_count += 1
                current_players[player] = [players_count, 0]
            self.setup_current_players()

        elif message.startswith("ATLEAST2PLAYERS"):
            msg = QMessageBox()
            msg.setWindowTitle("Błąd!")
            msg.setText("Do rozpoczęcia rozgrywki potrzeba minimum 2 graczy!")
            msg.exec_()

        elif message.startswith("START"):
            global sent
            self.labelCheckMyScore.setVisible(False)
            self.gameCheck[1].setVisible(False)
            self.gameCheck[2].setVisible(False)
            self.gameCheck[3].setVisible(False)
            self.gameCheck[4].setVisible(False)
            self.gameCheck[5].setVisible(False)
            self.gameCheck[6].setVisible(False)
            self.gameCheck[7].setVisible(False)
            self.gameCheck[8].setVisible(False)
            self.gameButtonStart.setVisible(False)
            letter = message.split("|")[1].strip()
            self.generatedLetter.setText(f"{letter}")
            self.sendButton.setVisible(True)
            self.time_remaining = 60
            self.timer.start(1000)
            sent = False
            ingameorno = False

        elif message.startswith("STOP"):
            self.ten_timer()



        elif message.startswith("WAITFORANSWERS"):
            msg = QMessageBox()
            msg.setWindowTitle("Cierpliwosci!")
            msg.setText("Jeszcze nie wszyscy gracze udzielili swoich odpowiedzi!")
            msg.exec_()

        elif message.startswith("THANKS"):
            who = message.split("|")[1]
            if my_nick == who:
                self.labelCheckMyScore.setVisible(True)
            else:
                player_ind, score = current_players[who]
                self.gameCheck[player_ind].setVisible(True)

        elif message.startswith("SCORES"):
            who = message.split("|")[1]
            score = int(message.split("|")[2])
            print(current_players)
            player_ind, current_points = current_players[who]
            if my_nick == who:
                self.gameLabelMyScore.setText(f"{who}: {score}")
                current_players[who] = [player_ind, score]
            else:
                self.gameScores[player_ind].setText(f"{who}: {score}")
                current_players[who] = [player_ind, score]

            self.countryTextEdit.setEnabled(True)
            self.cityTextEdit.setEnabled(True)
            self.nameTextEdit.setEnabled(True)
            self.sendButton.setEnabled(True)

        elif message.startswith("LEFT"):
            player = message.split("|")[1]
            player_ind, _= current_players[player]
            self.update_current_players(player)


        elif message.startswith("END"):
            msg = QMessageBox()
            msg.setWindowTitle("!!")
            msg.setText("Koniec gry, jesli chcesz zagrac ponownie dolacz do nowego pokoju!")
            msg.exec_()
            self.timer.stop()
            self.countryTextEdit.setEnabled(False)
            self.cityTextEdit.setEnabled(False)
            self.nameTextEdit.setEnabled(False)
            self.sendButton.setEnabled(False)
            self.labelCheckMyScore.setVisible(False)
            self.gameCheck[1].setVisible(False)
            self.gameCheck[2].setVisible(False)
            self.gameCheck[3].setVisible(False)
            self.gameCheck[4].setVisible(False)
            self.gameCheck[5].setVisible(False)
            self.gameCheck[6].setVisible(False)
            self.gameCheck[7].setVisible(False)
            self.gameCheck[8].setVisible(False)
            self.leave()



    def send_answers(self):
        global sent
        self.countryTextEdit.setDisabled(True)
        self.cityTextEdit.setDisabled(True)
        self.nameTextEdit.setDisabled(True)
        self.sendButton.setDisabled(True)

        country = self.countryTextEdit.toPlainText()
        city = self.cityTextEdit.toPlainText()
        name = self.nameTextEdit.toPlainText()
        if not country or not country.strip():
            country = "yyy"
        if not city or not city.strip():
            city = "yyy"
        if not name or not name.strip():
            name = "yyy"

        print(country,city,name)
        print(f"ANSWERS {country} {city} {name}")
        sent = True
        send_message("ANSWERS "+country.strip()+" "+city.strip()+" "+name.strip()+"\n\0")

    def ten_timer(self):
        if self.time_remaining > 10:
           self.time_remaining = 10

    def update_timer(self):
        global sent
        self.time_remaining -= 1
        self.timerLabel.setText(f"Time: {self.time_remaining} s")

        if self.time_remaining <= 0 and sent == False:
            self.timer.stop()
            self.send_answers()
        elif sent == True:
            self.timer.stop()

    def setup_current_players(self):
        global current_players
        for key, value in current_players.items():
            self.gameScores[value[0]].setText(f"{key}: 0")
            self.gameScores[value[0]].setVisible(True)


    def update_current_players(self, player):
        global players_count
        player_ind, _ = current_players[player]
        for key, value in current_players.items():
            self.gameScores[value[0]].setVisible(False)
        del current_players[player]
        players_count -= 1
        for key, value in current_players.items():
            if value[0] > player_ind:
                current_players[key] = [value[0]-1, value[1]]
        self.setup_current_players()

    def display_start(self):
        self.gameButtonStart.setVisible(True)

    def start_game(self):
        send_message("START"+"\n\0")



    def leave(self):
        global in_room
        global players_count
        current_players.clear()
        players_count = 0
        in_room = False
        send_message("LEAVE"+"\n\0")
        self.server_thread.terminate()
        self.hide()
        widget.removeWidget(self)
        lobby_screen = widget.widget(1)
        lobby_screen.update_combobox()
        widget.setCurrentIndex(widget.currentIndex())





if __name__ == "__main__":
    HOST = sys.argv[1]
    PORT = int(sys.argv[2])

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((HOST, PORT))

    app = QApplication(sys.argv)
    nick = NickScreen()
    widget = QStackedWidget()
    widget.addWidget(nick)
    widget.setFixedHeight(800)
    widget.setFixedWidth(1000)
    widget.show()

    try:
        sys.exit(app.exec_())
    except:
        pass
