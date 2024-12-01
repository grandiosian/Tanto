from krita import *
from PyQt5.QtWidgets import (QWidget, QPushButton, QVBoxLayout, QHBoxLayout, 
                            QLabel, QCheckBox, QLineEdit, QFileDialog, QGroupBox,
                            QMessageBox)
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QStyle
import os

DOCKER_ID = 'export_layers_docker'
DOCKER_TITLE = 'Export Layers'

class ExportLayersDocker(DockWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_TITLE)
        
        # Widget principal
        mainWidget = QWidget()
        
        # Tentative de définition de l'icône sur le widget principal
        #icon_path = os.path.join(os.path.dirname(__file__), "exportlayers.png")
        #if os.path.exists(icon_path):
        #    mainWidget.setWindowIcon(QIcon(icon_path))
        #    self.setWindowIcon(QIcon(icon_path))


        standardIcon = self.style().standardIcon(QStyle.SP_DialogSaveButton)
        self.setWindowIcon(standardIcon)
        
        #self.setWidget(mainWidget)
        
        # Création du widget principal
        mainWidget = QWidget()
        self.setWidget(mainWidget)
        mainLayout = QVBoxLayout()
        mainWidget.setLayout(mainLayout)
        
        # Description détaillée du script
        descriptionLabel = QLabel(
            "Organisation des calques pour l'export:\n\n"
            "1. DOSSIER_nomdudossier : crée un dossier 'nomdudossier'\n"
            "2. IMG_nomdelimage : groupe contenant les calques à exporter\n\n"
            "Important:\n"
            "- Mettez tous vos calques de dessin dans des groupes 'IMG_'\n"
            "- Ces groupes doivent être dans des dossiers 'DOSSIER_'\n"
            "- La hiérarchie doit être respectée:\n"
            "  DOSSIER_xxx/IMG_xxx/calques de dessin"
        )
        descriptionLabel.setWordWrap(True)
        mainLayout.addWidget(descriptionLabel)
        
        # Groupe pour les résolutions
        resolutionsGroup = QGroupBox("Résolutions à exporter")
        resolutionsLayout = QVBoxLayout()
        
        self.resolution_checkboxes = {}
        resolutions = [4098, 2048, 1024, 512, 256, 128, 64, 32, 16, 8]
        
        for res in resolutions:
            checkbox = QCheckBox(f"{res}px")
            if res in [1024, 512, 256, 128]:
                checkbox.setChecked(True)
            self.resolution_checkboxes[res] = checkbox
            resolutionsLayout.addWidget(checkbox)
        
        resolutionsGroup.setLayout(resolutionsLayout)
        mainLayout.addWidget(resolutionsGroup)
        
        # Groupe pour les options d'export
        exportOptionsGroup = QGroupBox("Options d'export")
        exportOptionsLayout = QVBoxLayout()
        
        # Option pour la transparence
        self.alphaCheckbox = QCheckBox("Activer la transparence")
        self.alphaCheckbox.setChecked(False)
        exportOptionsLayout.addWidget(self.alphaCheckbox)
        
        exportOptionsGroup.setLayout(exportOptionsLayout)
        mainLayout.addWidget(exportOptionsGroup)
        
        # Sélection du dossier d'export
        exportDirGroup = QGroupBox("Dossier d'export")
        exportDirLayout = QVBoxLayout()
        
        # Layout horizontal pour le chemin et le bouton parcourir
        pathLayout = QHBoxLayout()
        self.exportDirEdit = QLineEdit()
        browseButton = QPushButton("Parcourir...")
        browseButton.clicked.connect(self.browse_export_dir)
        pathLayout.addWidget(self.exportDirEdit)
        pathLayout.addWidget(browseButton)
        
        exportDirLayout.addLayout(pathLayout)
        
        # Bouton pour réinitialiser au dossier courant
        resetDirButton = QPushButton("Utiliser le dossier courant")
        resetDirButton.clicked.connect(self.reset_to_current_dir)
        exportDirLayout.addWidget(resetDirButton)
        
        exportDirGroup.setLayout(exportDirLayout)
        mainLayout.addWidget(exportDirGroup)
        
        # Bouton d'export
        exportButton = QPushButton("Exporter")
        exportButton.clicked.connect(self.export_layers)
        mainLayout.addWidget(exportButton)

    def canvasChanged(self, canvas):
        """Cette méthode est appelée quand le canvas change"""
        if canvas:
            self.setEnabled(True)
            if not self.exportDirEdit.text():
                doc = Krita.instance().activeDocument()
                if doc:
                    self.exportDirEdit.setText(os.path.dirname(doc.fileName()))
        else:
            self.setEnabled(False)

    def reset_to_current_dir(self):
        doc = Krita.instance().activeDocument()
        if doc:
            self.exportDirEdit.setText(os.path.dirname(doc.fileName()))

    def browse_export_dir(self):
        directory = QFileDialog.getExistingDirectory(self, "Sélectionner le dossier d'export")
        if directory:
            self.exportDirEdit.setText(directory)

    def show_success_message(self, nb_images):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Information)
        msg.setWindowTitle("Export réussi")
        msg.setText(f"Export terminé avec succès !\n{nb_images} images ont été exportées.")
        msg.exec_()

    def show_error_message(self, error_details):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setWindowTitle("Erreur d'export")
        msg.setText("Une erreur s'est produite pendant l'export.")
        msg.setDetailedText(str(error_details))
        msg.exec_()

    def hide_all_paint_layers(self, node):
        """Cache tous les calques de dessin qui ne sont pas dans des groupes IMG_"""
        for child in node.childNodes():
            if child.type() == "paintlayer":
                current_parent = child.parentNode()
                while current_parent:
                    if current_parent.name().startswith("IMG_"):
                        break
                    current_parent = current_parent.parentNode()
                if not current_parent:
                    child.setVisible(False)
            elif child.type() == "grouplayer":
                self.hide_all_paint_layers(child)

    def find_all_img_groups(self, root):
        """Trouve tous les groupes IMG_ dans l'arborescence"""
        img_groups = []
        if root.type() == "grouplayer":
            if root.name().startswith("IMG_"):
                img_groups.append(root)
            for child in root.childNodes():
                img_groups.extend(self.find_all_img_groups(child))
        return img_groups

    def toggle_img_groups_visibility(self, img_groups, active_group=None):
        """Cache tous les groupes IMG_ et active la visibilité du groupe actif et de ses parents"""
        # Cache d'abord tous les groupes IMG_
        for group in img_groups:
            group.setVisible(False)
        
        # Si on a un groupe actif, on active sa visibilité et celle de ses parents
        if active_group:
            current = active_group
            while current:
                current.setVisible(True)
                current = current.parentNode()

    def export_layers(self):
        try:
            exported_count = 0
            
            selected_resolutions = [
                res for res, checkbox in self.resolution_checkboxes.items() 
                if checkbox.isChecked()
            ]
            if not selected_resolutions:
                raise ValueError("Aucune résolution sélectionnée")
                
            selected_resolutions.sort(reverse=True)
            
            base_dir = self.exportDirEdit.text()
            if not base_dir:
                self.reset_to_current_dir()
                base_dir = self.exportDirEdit.text()
            
            if not os.path.exists(base_dir):
                raise ValueError(f"Le dossier d'export n'existe pas : {base_dir}")

            doc = Krita.instance().activeDocument()
            if doc is None:
                raise ValueError("Aucun document actif")

            def create_export_path(base_dir, folder_name):
                clean_name = folder_name.replace("DOSSIER_", "")
                path = os.path.join(base_dir, clean_name)
                if not os.path.exists(path):
                    os.makedirs(path)
                return path

            def export_image_group(group, save_dir, resolutions):
                nonlocal exported_count
                info = InfoObject()
                info.setProperty("alpha", self.alphaCheckbox.isChecked())
                info.setProperty("compression", 1)
                
                doc = Krita.instance().activeDocument()
                doc.setBatchmode(True)
                
                # Cache tous les calques de dessin hors des groupes IMG_
                self.hide_all_paint_layers(doc.rootNode())
                
                # Gère la visibilité des groupes IMG_
                all_img_groups = self.find_all_img_groups(doc.rootNode())
                self.toggle_img_groups_visibility(all_img_groups, group)
                
                original_width = doc.width()
                original_height = doc.height()
                
                image_name = group.name().replace("IMG_", "")
                
                for res in resolutions:
                    scale = res / original_width
                    width, height = res, int(original_height * scale)
                    
                    filename = f"{image_name}_{res}x{height}.png"
                    save_path = os.path.join(save_dir, filename)
                    
                    temp_doc = doc.clone()
                    temp_doc.setBatchmode(True)
                    temp_doc.scaleImage(width, height, 72, 72, "Bicubic")
                    temp_doc.exportImage(save_path, info)
                    temp_doc.close()
                    exported_count += 1

            def process_group(group, base_dir, resolutions):
                if group.name().startswith("DOSSIER_"):
                    export_dir = create_export_path(base_dir, group.name())
                    
                    for child in group.childNodes():
                        if child.type() == "grouplayer":
                            if child.name().startswith("IMG_"):
                                export_image_group(child, export_dir, resolutions)
                            else:
                                process_group(child, export_dir, resolutions)

            root_layers = doc.rootNode().childNodes()
            for layer in root_layers:
                if layer.type() == "grouplayer":
                    process_group(layer, base_dir, selected_resolutions)

            # Restaure la visibilité des groupes IMG_ à la fin
            all_img_groups = self.find_all_img_groups(doc.rootNode())
            for group in all_img_groups:
                group.setVisible(True)

            # Affiche le message de succès
            self.show_success_message(exported_count)

        except Exception as e:
            # En cas d'erreur, affiche le message d'erreur
            self.show_error_message(e)

class ExportLayersExtension(Extension):
    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        app = Krita.instance()
        # Création du docker sans le paramètre supplémentaire
        docker_widget_factory = DockWidgetFactory(
            DOCKER_ID,
            DockWidgetFactoryBase.DockRight,
            ExportLayersDocker
        )
            
        app.addDockWidgetFactory(docker_widget_factory)

    def createActions(self, window):
        pass
Application.addExtension(ExportLayersExtension(Application))