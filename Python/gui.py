import sys
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QPushButton, QVBoxLayout, QWidget

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('My HTML Page')

        # Create a central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Create a layout
        layout = QVBoxLayout()

        # Add widgets
        h1_label = QLabel('<h1>Hello, World!</h1>', self)
        layout.addWidget(h1_label)

        p_label = QLabel('<p>This is a simple HTML page.</p>', self)
        layout.addWidget(p_label)

        button = QPushButton('Click Me', self)
        layout.addWidget(button)

        # Set the layout to the central widget
        central_widget.setLayout(layout)

        # Apply the styles
        self.apply_styles()

        # Connect button click signal to a slot
        button.clicked.connect(self.on_button_click)

    def apply_styles(self):
        # Apply styles to the entire application
        self.setStyleSheet("""
            QWidget {
                background-color: black;
                color: red;
            }
            QLabel {
                color: red;
            }
            QPushButton {
                background-color: black;
                color: red;
                border: 1px solid red;
                padding: 5px;
            }
            QPushButton:hover {
                background-color: darkred;
            }
        """)

    def on_button_click(self):
        print('Button clicked!')

app = QApplication(sys.argv)
main_window = MainWindow()
main_window.show()
sys.exit(app.exec_())