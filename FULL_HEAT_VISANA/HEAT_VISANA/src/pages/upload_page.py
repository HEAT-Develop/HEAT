from PyQt6.QtWidgets import QLabel,QHBoxLayout,QLineEdit, QPushButton, QVBoxLayout, QWidget, QTextEdit, QComboBox,QFileDialog
from PyQt6.QtCore import Qt
from bs4 import BeautifulSoup
import requests
from urllib.parse import urljoin
from db.db import DataBase 
db_instance = DataBase()
dbInsert = db_instance.insert_new_mission
get_model = db_instance.get_model

import os
from pathlib import Path
from src.csv_vtk.csv_to_vtk import combine_vtk_csv 
from src.csv_vtk.create_a_new_csv import combine_vtk_csv as csv_long

visualType = {'49k','200k', '800k'}

class UploadPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Upload Page")
        self.initUI()
        self.model_in_dataBase()
       
    def initUI(self):
        layout = QVBoxLayout(self)
        
        
        top_layout = QHBoxLayout()
        
        self.choosemodel = QComboBox(self)
        self.choosemodel.currentIndexChanged.connect(self.update_model)
        self.choosemodel.addItems(visualType)
        
        self.button3 = QPushButton("Convert CSV to VTK", self)
        self.button3.clicked.connect(self.convert_csv_vtk)
        

        self.file_browse = QPushButton('Browse', self)
        self.file_browse.setGeometry(70, 0, 200, 50)
        self.file_browse.clicked.connect(lambda: self.open_file_dialog())

        self.filename_edit = QLineEdit()
        print("File name ",self.filename_edit)
        self.original_file_path = None 

        top_layout.addWidget(self.choosemodel)
        
        top_layout.addWidget(self.file_browse)
        top_layout.addWidget(self.button3)
        self.button = QPushButton("List VTK Files", self)
        self.button.clicked.connect(self.list_vtk_files)

        
        self.text_edit = QTextEdit(self)
        self.text_edit.setReadOnly(True)
        
        layout.addLayout(top_layout) 

        layout.addWidget(self.button)
        layout.addWidget(self.text_edit)

        self.setLayout(layout)

        self.model = ""
        self.file_name =""
        self.chooseModel_csv =""
        self.new_csv_file_name = ""
        self.new_vtk_file_name = ""

    def model_in_dataBase(self):
        model_spc_49k = "Application_code/src/models/spc_49k_model.vtk"
        model_spc_200k = "Application_code/src/models/spc_200k_model.vtk"
        model_spc_800k = "Application_code/src/models/spc_800k_model.vtk"
        dic = {model_spc_49k:"spc_49k_model.vtk",
               model_spc_200k:"spc_200k_model.vtk",
               model_spc_800k:"spc_800k_model.vtk" }

        for file_path ,model_name in dic.items():
            filename = os.path.basename(model_name)
            name, ext = os.path.splitext(filename) 

            if not db_instance.model_in_db(name):
                print(f"Adding model {model_name} to the database.")
                base_directory = os.path.join(os.path.dirname(__file__), 'files')
                os.makedirs(base_directory, exist_ok=True)
                absolute_file_path = os.path.abspath(file_path)
                print("Absolute FIle Path: ", absolute_file_path)

                
               
                name_parts = name.split("_")

                print("File Name: ", filename)
                if len(name_parts)>2:
                    specification = name_parts[0]
                    size = name_parts[1]
                else:
                    print(f"Warning: Unexpected filename format: {filename}")
                    continue
                
                asteroid = "Ryugu"
                type_file = ext.lstrip(".")

                

                db_instance.insert_new_model(name, asteroid, type_file, specification, absolute_file_path, size)    
    
    def list_vtk_files(self):
        base_url = 'https://data.darts.isas.jaxa.jp/pub/hayabusa2/tir_bundle/data_map/proximity/'
        self.text_edit.clear()
        self.get_vtk_files(base_url)

    def get_vtk_files(self, url, depth=3):
        visited_urls = set()
        headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3'
        }

        def fetch_files(current_url, current_depth):
            if current_depth <= 0:
                return

            visited_urls.add(current_url)
            try:
                print(f"Fetching URL: {current_url}")
                response = requests.get(current_url, headers=headers)

                if response.status_code == 403:
                    print(f"Access forbidden to URL: {current_url}. Changing folder.")
                    next_folder = self.get_next_folder(current_url)
                    if next_folder:
                        fetch_files(next_folder, current_depth)
                    return
                
                response.raise_for_status()
                print(f"Response Status Code: {response.status_code}")
                soup = BeautifulSoup(response.text, 'html.parser')
                links = [a['href'] for a in soup.find_all('a', href=True)]

                for link in links:
                    full_url = urljoin(current_url, link)
                    if link.endswith('/'):
                        fetch_files(full_url, current_depth - 1)
                    elif link.endswith('.vtk'):
                        self.try_insert_file(full_url)

            except requests.RequestException as e:
                print(f"Request Error: {e}")
                if hasattr(e, 'response') and e.response is not None:
                    print(f"Response Text: {e.response.text}")

        fetch_files(url, depth)

    def get_next_folder(self, current_url):
        try:
            response = requests.get(current_url)
            response.raise_for_status()
            soup = BeautifulSoup(response.text, 'html.parser')
            links = [a['href'] for a in soup.find_all('a', href=True) if a['href'].endswith('/')]

            if links:
                next_folder = urljoin(current_url, links[0])
                print(f"Found next folder: {next_folder}")
                return next_folder
            else:
                print("No further folders found.")
                return None
        except requests.RequestException as e:
            print(f"Error fetching folders: {e}")
            return None

    
    def try_insert_file(self, file_url):
        session = requests.Session()
        session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3',
            'Referer': 'https://data.darts.isas.jaxa.jp/pub/hayabusa2/tir_bundle/data_map/proximity/'
        })

        try:
            response = session.get(file_url)
            if response.status_code == 200:
                filename = file_url.split("/")[-1]
                base_directory = os.path.join(os.path.dirname(__file__), 'files')

                temp_path = os.path.join(base_directory, filename)
                os.makedirs(base_directory, exist_ok=True)
                
                with open(temp_path, 'wb') as file:
                    file.write(response.content)

                with open(temp_path, 'r') as file:
                    lines = file.readlines()
                    if len(lines) > 1:
                        second_line = lines[1]
                    else:
                        print("File does not contain a second line.")
                        return  
                
                filedata = filename.split("_")
                mission = filedata[0]
                camera = filedata[1]
                year = filedata[2][:4]
                month = filedata[2][4:6]
                day = filedata[2][6:8]
                hour = filedata[3][:2]
                minute = filedata[3][2:4]
                sec = filedata[3][4:6]
                file_format = filedata[4].split(".")[1]
                
                # Parse model and size
                model = second_line.split("_")[2]
                if model not in ["sfm", "spc"]:
                    model = "spc"
                
                model_size = second_line.split("_")[3]
                valid_sizes = {"sfm": ["50k", "200k", "800k"], "spc": ["49k", "200k", "800k"]}
                if model_size not in valid_sizes[model]:
                    model_size = second_line.split("_")[2]

                model_directory = os.path.join(base_directory, model, model_size)
                os.makedirs(model_directory, exist_ok=True)

                local_path = os.path.join(model_directory, filename)
                os.rename(temp_path, local_path)

                level = filedata[4].split(".")[0]

                dbInsert(filename, level, mission, camera, year, month, day, hour, minute, sec, file_format, model, local_path, model_size)

            elif response.status_code == 403:
                print(f"Access forbidden to file: {file_url}")
            else:
                print(f"Failed to download {file_url}. Status code: {response.status_code}")
        except requests.RequestException as e:
            print(f"Request Error for file {file_url}: {e}")
            if hasattr(e, 'response') and e.response is not None:
                print(f"Response Text: {e.response.text}")


    def convert_csv_vtk(self):
     
        
        base_name = os.path.basename(self.new_csv_file_name)
        vtk_file_name, csv_file_extension_not_usful = os.path.splitext(base_name)
        vtk_paths = "Application_code/src/files/vtk_converted_files/"
        self.new_vtk_file_name = f"{vtk_paths}{vtk_file_name}_{self.chooseModel_csv}.vtk"
    
        combine_vtk_csv(self.model, self.new_csv_file_name,self.new_vtk_file_name )

    def update_model(self,i):
        self.model = get_model(self.choosemodel.currentText())
        self.chooseModel_csv = self.choosemodel.currentText()
        print(self.model)
        for count in range(self.choosemodel.count()):
            print (self.choosemodel.itemText(count))

    try: 
        def open_file_dialog(self):
            filename, ok = QFileDialog.getOpenFileName(
                self,
                "Select a File",
                "CSV Files (*.csv)"
            )
            if filename:
                path = Path(filename)
                self.filename_edit.setText(str(path))
            
            print("fileName",filename)
            base_name = os.path.basename(filename)
            print("Base Nmae: ", base_name)

            file_name_csv, file_extension = os.path.splitext(base_name)
            print("File csv name: ",file_name_csv)

            paths = "Application_code/src/files/csvfiles/full_csv/"
            self.new_csv_file_name = f"{paths}{file_name_csv}_{self.chooseModel_csv}.csv"
            print("New csv name:", self.new_csv_file_name)
            csv_long(filename,self.new_csv_file_name , self.chooseModel_csv)
            self.file_name = filename
            print(filename)
    except TypeError as e:
        print("Error while connecting to MySQL:", e)