from PyQt6.QtWidgets import QApplication, QMainWindow, QToolBar, QStackedWidget, QSpacerItem, QSizePolicy, QWidget, QHBoxLayout, QToolButton
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import Qt,QSize
import sys
import os
import pyvista
from src.pages.analysis_page import AnalysisPage
from src.pages.time_page import TimePage
from src.pages.multiview_page import MultiviewPage
from src.pages.upload_page import UploadPage

from src.pages.analysis_folder.rigions import Regions

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Visualization in 3D")
        self.setGeometry(100, 100, 1260, 900)

        # Central widget with QStackedWidget
        self.central_widget = QStackedWidget(self)
        self.setCentralWidget(self.central_widget)

        self.page1 = AnalysisPage(self)
        self.page2 = TimePage(self)
        self.page3 = MultiviewPage(self)
        self.page4 = UploadPage(self)

        self.central_widget.addWidget(self.page1)
        self.central_widget.addWidget(self.page2)
        self.central_widget.addWidget(self.page3)
        self.central_widget.addWidget(self.page4)

        bottom_toolbar = QToolBar("Bottom Toolbar", self)
        bottom_toolbar.setMovable(False)
        bottom_toolbar.setOrientation(Qt.Orientation.Horizontal)
        bottom_toolbar.setStyleSheet("""
            QToolBar { 
                background-color: rgba(0, 125, 255, 1);
                spacing: 100px;
                color: black;
                border-radius: 0px;
                height: 25px;
            }
            QToolButton {
                background-color: rgba(0, 125, 255, 1);
                color: white;
                border: none; 
                margin-right: 80px; 
                margin-left: 80px; 
                border-radius: 0px; 
                min-width: 20px; /* Set a minimum width for buttons */
                min-height: 20px; /* Set a minimum height for buttons */
            }
             
        """)

        button_widget = QWidget()
        button_layout = QHBoxLayout()

        left_spacer = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        right_spacer = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)
        
        button_layout.addItem(left_spacer)

        script_dir = os.path.dirname(os.path.abspath(__file__))
        icon_dir = os.path.join(script_dir, "src", "images")

        icon_size = 30  

        home_action = QAction(QIcon(os.path.join(icon_dir, "analisis.png")), "Analysis", self)
        search_action = QAction(QIcon(os.path.join(icon_dir, "time.png")), "Time", self)
        add_action = QAction(QIcon(os.path.join(icon_dir, "multiviewer.png")), "Multiview", self)
        activity_action = QAction(QIcon(os.path.join(icon_dir, "upload.png")), "Upload", self)

        home_action.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page1))
        search_action.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page2))
        add_action.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page3))
        activity_action.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page4))

        home_button = QToolButton()
        home_button.setDefaultAction(home_action)
        home_button.setIconSize(QSize(icon_size, icon_size)) 
        button_layout.addWidget(home_button)

        search_button = QToolButton()
        search_button.setDefaultAction(search_action)
        search_button.setIconSize(QSize(icon_size, icon_size)) 
        button_layout.addWidget(search_button)

        add_button = QToolButton()
        add_button.setDefaultAction(add_action)
        add_button.setIconSize(QSize(icon_size, icon_size))  
        button_layout.addWidget(add_button)

        activity_button = QToolButton()
        activity_button.setDefaultAction(activity_action)
        activity_button.setIconSize(QSize(icon_size, icon_size)) 
        button_layout.addWidget(activity_button)

        button_layout.addItem(right_spacer)
        button_widget.setLayout(button_layout)
        bottom_toolbar.addWidget(button_widget)
        self.addToolBar(Qt.ToolBarArea.BottomToolBarArea, bottom_toolbar)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyleSheet("""
        QMainWindow {
            background-color: white;  
            color: black;  
        }
    """)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
