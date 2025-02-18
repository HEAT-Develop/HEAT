from PyQt6.QtWidgets import QVBoxLayout, QHBoxLayout,QPushButton, QWidget, QComboBox, QGridLayout, QSizePolicy, QLabel, QDateTimeEdit, QSpacerItem
from PyQt6.QtCore import Qt, QDateTime
import pyvista as pv
from pyvistaqt import BackgroundPlotter
from db.db import DataBase
db_instance = DataBase()

from datetime import datetime
import os

visualType = {'brightness_temperature','emission_angle', 'incidence_angle', 'phase_angle','local_solar_time_normal_vector'}
data = ""

class PyVistaWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.layout = QVBoxLayout(self)
        self.setLayout(self.layout)
        
        # Create a PyVista plotter and embed it into the QWidget
        self.plotter = BackgroundPlotter(show=False)
        self.plotter.view_vector([1, 1, 1])
        self.layout.addWidget(self.plotter.interactor)

        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.layout.setContentsMargins(0, 0, 0, 0)
        self.layout.setSpacing(0)

    def update_plot(self, mesh,d):
        """ Update the PyVista plot. """
        self.plotter.clear()
        self.plotter.add_mesh(mesh,scalars = d)
        print("update plot",d)
        self.plotter.update()
        
 
class MultiviewPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Multiview Page")
        self.views = []  # Store references to PyVistaWidget instances
        self.initUI()

    def initUI(self):
    
        
        self.load_css('Application_code/src/style/multiview.css') 
        print(os.path.abspath('src/style/multiview.css'))
        
        layout = QVBoxLayout(self)
        top_layout = QHBoxLayout()
        self.selectView = QComboBox(self)
        self.selectView.addItem("How many views")
        self.selectView.setObjectName("selectViewComboBox")
        self.selectView.addItems([str(i) for i in range(1, 10)])  
        self.selectView.setFixedWidth(150)  # Set the width of the combo box
        self.selectView.currentIndexChanged.connect(self.selectionchange)
        top_layout.addWidget(self.selectView, alignment=Qt.AlignmentFlag.AlignHCenter)

        middle_layout = QVBoxLayout()
        self.plotter_widget = QWidget(self)
        self.grid_layout = QGridLayout(self.plotter_widget)
        self.plotter_widget.setLayout(self.grid_layout)
        
        layout.addLayout(top_layout)
        layout.addWidget(self.plotter_widget)
        self.setLayout(layout)

        self.file =""

    def load_css(self, file_path):
        try:
            with open(file_path, 'r') as css_file:
                css = css_file.read()
                self.setStyleSheet(css)  # Apply the CSS styles
        except FileNotFoundError:
            print(f"Error: The file {file_path} was not found.")
        except Exception as e:
            print(f"Error loading CSS: {e}")   
    
    def selectionchange(self, i):
        if i > 0:  # Ignore the first item "How many views"
            num_views = int(self.selectView.currentText())
            self.update_views(num_views)

    def get_grid_dimensions(self, num):
        if num <= 3:
            return 1, num
        elif num <= 6:
            return 2, (num + 1) // 2
        elif num <= 9:
            return 3, 3
        else:
            raise ValueError("Unsupported number of views")
        
    def update_views(self, num):
        for i in reversed(range(self.grid_layout.count())):
            widget = self.grid_layout.itemAt(i).widget()
            if widget:
                widget.deleteLater()

        self.views.clear()

        rows, cols = self.get_grid_dimensions(num)

        for i in range(self.grid_layout.rowCount()):
            self.grid_layout.setRowStretch(i, 0)
        for i in range(self.grid_layout.columnCount()):
            self.grid_layout.setColumnStretch(i, 0)

        for row in range(rows):
            self.grid_layout.setRowStretch(row, 1)  
        for col in range(cols):
            self.grid_layout.setColumnStretch(col, 1)  

        for row in range(rows):
            for col in range(cols):
                index = row * cols + col
                if index < num:

                    container = QWidget()
                    container_layout = QVBoxLayout(container)
                    container_layout.setContentsMargins(0, 0, 0, 0)

                    header_layout = QHBoxLayout()
                    date_time_edit = QDateTimeEdit(QDateTime.currentDateTime())
                    send_date = QPushButton('Set Date', self)

                    chooseVisualization = QComboBox(self)

                    self.choose_model = QComboBox(self)
                    self.choose_model.addItems(["sfm", "spc"]) 
                    self.choose_model.currentIndexChanged.connect(self.update_model_size)  

                    self.choose_model_size = QComboBox(self)


                    date_time_edit.setDisplayFormat("yyyy-MM-dd HH:mm:ss")
                    date_time_edit.setCalendarPopup(True)
                    all_dates, dates_range = self.set_valid_dates(date_time_edit)
                    min_date = dates_range[0]
                    max_date = dates_range[-1]
                    date_time_edit.setMinimumDate(min_date)
                    date_time_edit.setMaximumDate(max_date)

                    allowed_dates = all_dates

                    date_time_edit.dateTimeChanged.connect(lambda _, idx=index, edit=date_time_edit: self.check_date(idx, edit))

                    header_layout.addWidget(date_time_edit)
                    header_layout.addWidget(self.choose_model)
                    header_layout.addWidget(self.choose_model_size)
                    header_layout.addWidget(send_date)
                    header_layout.addWidget(chooseVisualization)
                    container_layout.addLayout(header_layout)

                    view = PyVistaWidget()
                    container_layout.addWidget(view)

                    self.views.append({
                        'plot_widget': view,
                        'date_time_edit': date_time_edit,
                        'choose_model': self.choose_model,
                        'choose_model_size':self.choose_model_size,
                        'send_date': send_date,
                        'chooseVisualization': chooseVisualization,
                        'allowed_dates': allowed_dates,
                        'mesh': None
                    })

                    self.grid_layout.addWidget(container, row, col)

                    send_date.clicked.connect(lambda _, idx=index: self.handle_datetime_change(idx))
                    chooseVisualization.currentIndexChanged.connect(lambda _, idx=index: self.updateVisualizationType(idx))
                    self.update_model_size()

    def update_model_size(self):
        self.choose_model_size.clear()
        if self.choose_model.currentText() == "sfm":
            self.choose_model_size.addItems(["50k", "200k", "800k"])
        elif self.choose_model.currentText() == "spc":
            self.choose_model_size.addItems(["49k", "200k", "800k"])

    def check_date(self, idx, date_time_edit):
        selected_date = date_time_edit.dateTime()
        selected_date_str = selected_date.toString("yyyy-MM-dd HH:mm:ss")

        allowed_dates = self.views[idx]['allowed_dates']
        allowed_dates_qdatetime = [QDateTime.fromString(d, "yyyy-MM-dd HH:mm:ss") for d in allowed_dates]

        if selected_date not in allowed_dates_qdatetime:
            nearest_date = min(allowed_dates_qdatetime, key=lambda d: abs(selected_date.secsTo(d)))
            
            date_time_edit.setDateTime(nearest_date)

    def set_valid_dates(self, date_time_edit):
            dates_complex= db_instance.get_dates()
            dates_simple = db_instance.get_dates_day()
            dates =[]
            dates2 = []
            for d in dates_complex:
                parsed_date = datetime.strptime(d, '%Y-%m-%d %H:%M:%S')

                dates.append(parsed_date)
            
            for d in dates_simple:
                parsed_date = datetime.strptime(d, '%Y-%m-%d')

                dates2.append(parsed_date)
            return dates_complex, dates2
            
    def handle_datetime_change(self, view_index):
        view = self.views[view_index] 
        datetime = view['date_time_edit'].dateTime()
        year = datetime.date().year()
        month = datetime.date().month()
        day = datetime.date().day()
        hour = datetime.time().hour()
        minute = datetime.time().minute()
        second = datetime.time().second()

        model = view['choose_model'].currentText()
        model_size = view['choose_model_size'].currentText()
        print("minutes: ", minute)
        # Fetch the appropriate file based on the date and time
        file = db_instance.get_name(year, month, day, hour, minute, second, model, model_size)
        
        if file:
            view['mesh'] = pv.read(file[0]) 
            self.setupVisualizationType(view_index)  

    def setupVisualizationType(self, view_index):
        """ Populate visualization types for a specific view after loading the file and mesh. """
        view = self.views[view_index]

        if view['mesh']:
            visual = view['mesh'][0].array_names
            exclude = ['longitude', 'latitude', 'pixel_x', 'pixel_y']
            view['chooseVisualization'].clear()  # Clear existing items
            for types in visual:
                if types not in exclude:
                    view['chooseVisualization'].addItem(types)

    def updateVisualizationType(self, view_index):
        view = self.views[view_index]

        # Get the selected visualization type for the current view
        selected_visual_type = view['chooseVisualization'].currentText()

        if view['mesh'] and selected_visual_type:
            # Update only this view's plot
            view['plot_widget'].update_plot(view['mesh'], selected_visual_type)
