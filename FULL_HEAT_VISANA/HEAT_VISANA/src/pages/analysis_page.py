from PyQt6.QtWidgets import QApplication, QMainWindow, QToolBar, QStackedWidget, QSpacerItem, QSizePolicy, QWidget, QHBoxLayout, QToolButton
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import Qt,QSize
import os

from src.pages.analysis_folder.rigions import Regions
from src.pages.analysis_folder.selection import Selections

class AnalysisPage(QMainWindow):
    def __init__(self, parent = None):
        super().__init__(parent)

        self.setWindowTitle("Visualization in 3D")
        self.setGeometry(100, 100, 1260, 900)

        # Central widget with QStackedWidget
        self.central_widget = QStackedWidget(self)
        self.setCentralWidget(self.central_widget)

        self.page3 = Regions(self)
        self.page1 = Selections(self)

        self.central_widget.addWidget(self.page1)
        #self.central_widget.addWidget(self.page2)
        self.central_widget.addWidget(self.page3)

        bottom_toolbar = QToolBar("Bottom Toolbar", self)
        bottom_toolbar.setMovable(False)
        bottom_toolbar.setOrientation(Qt.Orientation.Horizontal)
        bottom_toolbar.setStyleSheet("""
            QToolBar { 
                background-color: rgba(0, 125, 255, 1);
                spacing: 20px;
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

        script_dir = "FULL_HEAT_VISANA/HEAT_VISANA"
        icon_dir = os.path.join(script_dir, "src", "images")
        print(script_dir)
        icon_size = 30  # Change this size as necessary

        #plot_over_line = QAction(QIcon(os.path.join(icon_dir, "plot_line.png")), "Plot Line", self)
        select_region = QAction(QIcon(os.path.join(icon_dir, "multi_section.png")), "Multi Section", self)
        selection = QAction(QIcon(os.path.join(icon_dir, "slection.png")), "Slection", self)

        select_region.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page1))
        #plot_over_line.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page2))
        selection.triggered.connect(lambda: self.central_widget.setCurrentWidget(self.page3))

        
        search_button = QToolButton()
        search_button.setDefaultAction(select_region)
        search_button.setIconSize(QSize(icon_size, icon_size))  # Set the icon size
        button_layout.addWidget(search_button)

        add_button = QToolButton()
        add_button.setDefaultAction(selection)
        add_button.setIconSize(QSize(icon_size, icon_size))  # Set the icon size
        button_layout.addWidget(add_button)

        button_layout.addItem(right_spacer)
        button_widget.setLayout(button_layout)
        bottom_toolbar.addWidget(button_widget)
        self.addToolBar(Qt.ToolBarArea.TopToolBarArea, bottom_toolbar)