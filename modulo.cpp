/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "modulo.h"
#include <kaboutdata.h>
#include <KGenericFactory>
#include <KPluginFactory>
#include <QtGui>


// Declaracion del plugin
K_PLUGIN_FACTORY(ModuloFactory, registerPlugin<Modulo>();)
K_EXPORT_PLUGIN(ModuloFactory("cgc"))

Modulo::Modulo(QWidget* parent, const QVariantList& args):
KCModule(ModuloFactory::componentData(), parent),
ui(new Ui::GUI)
{
    
       KAboutData *acercade = new KAboutData("cgc","kcm_cgc",ki18n("Chakra Gtk Config"),"1.3",
                      ki18n("Config your Gtk Apps"),
                      KAboutData::License_GPL_V2,
                      ki18n("Copyright 2011 José Antonio Sánchez Reynaga"));
       acercade->addAuthor(ki18n("José Antonio Sánchez Reynaga (antonioJASR)"),ki18n("Programador"), "joanzare@gmail.com");
       acercade->addCredit(ki18n("Manuel Tortosa (manutortosa)"), ki18n("Aportador de ideas, tester y traductor"));
       acercade->addCredit(ki18n("Adrián Chávez Hernández (Gallaecio)"), ki18n("traduccion"));
       setAboutData(acercade);
    
    ui->setupUi(this);
    appareance = new AparienciaGTK;
    refreshLists();
    makePreviewIconTheme();
    makePreviewFont();
    
    
      //evento para aplicar cambios

    //evento que notifican que hubo cambios en la interfaz grafica
    connect(ui->cb_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(appChanged()));
    connect(ui->cb_icon, SIGNAL(currentIndexChanged(int)), this, SLOT(appChanged()));
    connect(ui->cb_icon_fallback ,SIGNAL(currentIndexChanged(int)), this, SLOT(appChanged()));
    connect(ui->cb_font, SIGNAL(currentIndexChanged(int)), this, SLOT(appChanged()));
    connect(ui->cb_font_style, SIGNAL(currentIndexChanged(int)), this, SLOT(appChanged()));
    connect(ui->spin_font_tam, SIGNAL(valueChanged(int)), this, SLOT(appChanged()));

    //eventos para hacer previews
    connect(ui->cb_icon_fallback, SIGNAL(activated(QString)), this, SLOT(makePreviewIconTheme()));
    connect(ui->cb_icon, SIGNAL(activated(QString)), this, SLOT(makePreviewIconTheme()));


    //EVENTOS PARA LOS BOTONES DE LOS INSTALADORES
    connect(ui->clb_add_theme, SIGNAL(clicked(bool)), this, SLOT(showDialogForInstall()));
    connect(ui->clb_remove_theme, SIGNAL(clicked(bool)), this, SLOT(showDialogForUninstall()));
    
    installer =  new DialogInstaller(this);
    uninstaller = new DialogUninstaller(this, appareance);
    
    
    //PREVISUALIZACION DEL TIPO DE LETRA
    connect(ui->cb_font, SIGNAL(activated(QString)), this, SLOT(makePreviewFont()));
    connect(ui->cb_font_style, SIGNAL(activated(QString)), this, SLOT(makePreviewFont()));
    connect(ui->spin_font_tam, SIGNAL(valueChanged(int)), this, SLOT(makePreviewFont()));
}


Modulo::~Modulo()
{
    delete ui;    
    delete appareance;
    delete installer;
    delete uninstaller;
}


void Modulo::refreshLists()
{

    //Alcualizamos el combobox de los temas
    ui->cb_theme->clear(); //limpiamos checkbox
    ui->cb_theme->addItems(appareance->getAvaliableThemes());

    //Actualizamos el combox de los temas de iconos disponibles
    ui->cb_icon->clear(); // limpiamos checkbox
    ui->cb_icon->addItems(appareance->getAvaliableIcons());

    //Actualizamos el combobox de los temas de iconos fallback disponibles
    ui->cb_icon_fallback->clear();
    ui->cb_icon_fallback->addItems(appareance->getAvaliableIcons());

    //Acualizamos los comboboxs con el tema que esta definido desde el archiovo .gtkrc-2.0

    //combobox del tema
    if(!appareance->getTheme().isEmpty()){
        int pos= ui->cb_theme->findText(appareance->getTheme());
        ui->cb_theme->setCurrentIndex(pos);
    }

    //combobox de icono
    if(!appareance->getIcon().isEmpty()){
        int  pos = ui->cb_icon->findText(appareance->getIcon());
        ui->cb_icon->setCurrentIndex(pos);
    }

    //combobox de icono fallback
    if(!appareance->getIconFallBack().isEmpty()){
        int pos = ui->cb_icon_fallback->findText(appareance->getIconFallBack());
        ui->cb_icon_fallback->setCurrentIndex(pos);
    }


    //Trabajamos con la fuente

    QString fuente = appareance->getFont();

    if(!fuente.isEmpty()){

        bool negrita = false, cursiva = false;

        //buscamos bold en la cadena y la reemplazamos
        if(fuente.contains(QRegExp("bold "))){
            fuente.replace(QRegExp("bold "), "");
            negrita = true;
        }

        if(fuente.contains(QRegExp("italic "))){
            fuente.replace(QRegExp("italic "), "");
            cursiva = true;
        }

        //Buscamos la posicion  de algun valor numerico
        int pos = fuente.lastIndexOf(QRegExp(QString("([0-9][0-9]|[0-9])")))-1;
        QString num = fuente.mid(pos); //encontramos los pixeles de la fuente
        QString nombre = fuente.mid(0, pos-1); //Encontramos el nombre de la fuente

        ui->cb_font->setCurrentFont(QFont(nombre));

        //Establecemos el estilo de la fuente
        if(negrita && !cursiva)
            pos = 1;
        else if(!negrita && cursiva)
            pos = 2;
        else if(negrita && cursiva)
            pos = 3;
        else
            pos = 0;

        ui->cb_font_style->setCurrentIndex(pos);

        //Establecemos el tamaño
        ui->spin_font_tam->setValue(num.toInt());

        qDebug() << "CURRENT FOUNT:" << fuente << "\n"
               << "num : " << num << " , nombre : " << nombre;

    }

}

//Ofrece una vista previa de iconos
void Modulo::makePreviewIconTheme()
{
    //Buscar primero los iconos del tema fallback
    //file:///usr/share/icons/oxygen-refit-2-2.5.0/48x48/places/folder_home.png
    //file:///usr/share/icons/oxygen-refit-2-2.5.0/48x48/places/user-trash.png
    //file:///usr/share/icons/oxygen-refit-2-2.5.0/48x48/actions/document-print.png

    //Borramos iconos anteriores
    ui->lb_prev_1->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_2->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_3->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_4->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_5->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_6->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_7->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_8->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    ui->lb_prev_9->setPixmap(QPixmap("/usr/share/icons/oxygen/48x48/mimetypes/application-x-zerosize.png"));
    

    

    //BUSCAMOS LA RUTA DEL TEMA DE ICONOS INSTALADO
    QString icono_fallback = ui->cb_icon_fallback->currentText();
    QString path_fallback;

    foreach(QString i, appareance->getAvaliableIconsPaths()){
        if(i.contains(icono_fallback)){
            path_fallback   = i;
            break;
        }
    }

    qDebug() << "-- URL PATH ICON FALLBACK " << path_fallback;


    //PRIMER ICONO

    QFileInfo temp(path_fallback+"/48x48/places/folder_home.png");
    qDebug() << "******************************** FALLBACK ICONS"
             << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();

    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/folder_home.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    
    //Soporte para elementary
    temp.setFile(path_fallback+"/places/48/folder_home.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));
    

    //SEGUNDO ICONO

    temp.setFile(path_fallback+"/places/48/user-trash.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/user-trash.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_fallback+"/48x48/places/user-trash.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    

    //TERCER ICONO

    temp.setFile(path_fallback+"/48x48/actions/document-print.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/actions/48/document-print.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_fallback+"/actions/48/document-print.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));

        
    //CUARTO ICONO
    temp.setFile(path_fallback+"/48x48/places/user-desktop.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/user-desktop.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_fallback+"/places/48/user-desktop.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //CINCO ICONO
    temp.setFile(path_fallback+"/48x48/places/user-bookmarks.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/user-bookmarks.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));
    
     temp.setFile(path_fallback+"/places/48/user-bookmarks.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    qDebug() << "\n***********************************************";
    
    //SEXTO ICONO
    temp.setFile(path_fallback+"/48x48/places/network-server.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/network-server.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_fallback+"/places/48/network-server.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //SEPTIMO ICONO
    temp.setFile(path_fallback+"/48x48/apps/help.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/apps/48/help.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));

    
    temp.setFile(path_fallback+"/apps/48/help.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //OCTAVO ICONO
    temp.setFile(path_fallback+"/48x48/places/start-here.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/places/48/start-here.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));
    
        temp.setFile(path_fallback+"/places/48/start-here.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //NOVENO ICONO

    temp.setFile(path_fallback+"/48x48/actions/up.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_fallback+"/actions/48/up.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));
    
       temp.setFile(path_fallback+"/actions/48/up.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";


    //AHORA TOCA EL TURNO DE LOS ICONOS
    //BUSCAMOS LA RUTA DEL TEMA DE ICONOS INSTALADO
    QString icono = ui->cb_icon->currentText();
    QString path_icon;

    foreach(QString i, appareance->getAvaliableIconsPaths()){
        if(i.contains(icono)){
            path_icon  = i;
            break;
        }
    }

    qDebug() << "-- URL ICON PATH " << path_icon;


    //PRIMER ICONO

    temp.setFile(path_icon+"/48x48/places/folder_home.png");
    qDebug() << "******************************** NORMAL ICONS"
             << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();

    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/folder_home.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));
    
      temp.setFile(path_icon+"/places/48/folder_home.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_1->setPixmap(QPixmap(temp.absoluteFilePath()));

    //SEGUNDO ICONO

    temp.setFile(path_icon+"/places/48/user-trash.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));

    
        temp.setFile(path_icon+"/places/48/user-trash.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_icon+"/48x48/places/user-trash.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_2->setPixmap(QPixmap(temp.absoluteFilePath()));

    //TERCER ICONO

    temp.setFile(path_icon+"/48x48/actions/document-print.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/actions/48/document-print.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));
    
     temp.setFile(path_icon+"/actions/48/document-print.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_3->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //CUARTO ICONO
    temp.setFile(path_icon+"/48x48/places/user-desktop.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/user-desktop.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/user-desktop.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_4->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    qDebug() << "\n***********************************************";
    
    //CINCO ICONO
    temp.setFile(path_icon+"/48x48/places/user-bookmarks.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/user-bookmarks.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_icon+"/places/48/user-bookmarks.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_5->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //SEXTO ICONO
    temp.setFile(path_icon+"/48x48/places/network-server.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/network-server.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_icon+"/places/48/network-server.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_6->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //SEPTIMO ICONO
    temp.setFile(path_icon+"/48x48/apps/help.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/apps/48/help.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));
    
    temp.setFile(path_icon+"/apps/48/help.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_7->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //OCTAVO ICONO
    temp.setFile(path_icon+"/48x48/places/start-here.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/places/48/start-here.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));
    
        temp.setFile(path_icon+"/places/48/start-here.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_8->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
    
    //NOVENO ICONO

    temp.setFile(path_icon+"/48x48/actions/up.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));

    temp.setFile(path_icon+"/actions/48/up.png");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));
    
     temp.setFile(path_icon+"/actions/48/up.svg");
    qDebug() << "\nExist : " << temp.absoluteFilePath() << " : " << temp.exists();
    if(temp.exists()) ui->lb_prev_9->setPixmap(QPixmap(temp.absoluteFilePath()));
    qDebug() << "\n***********************************************";
}

void Modulo::appChanged()
{
   //Habilitamos el boton de instalar
   emit(changed(true));
}



// SOBRECARGA DE METODOS DE KCModule
void Modulo::save()
{
    
    //Establecemos los valores para guardarlo en el archivo .gtkrc-2.0
    appareance->setTheme(ui->cb_theme->currentText());

    //hay que encontrar el path del tema
    foreach(QString i, appareance->getAvaliableThemesPaths()){
        if(i.contains(ui->cb_theme->currentText())){
            appareance->setThemePath(i);
            break;
        }
    }


    appareance->setIcon(ui->cb_icon->currentText());
    appareance->setIconFallBack(ui->cb_icon_fallback->currentText());

    //Establecemos valores den negrita o cursuva
    QString estilo;
    int pos = ui->cb_font_style->currentIndex();

    if(pos == 1)
        estilo = "bold";
    else if(pos == 2)
        estilo = "italic";
    else if(pos == 3)
        estilo = "bold italic";
    else
        estilo = "";

    QString fuente;
    if(estilo.isEmpty()){
        //Esto hace que tenga menos espacios en blanco entre la fuente y el tamaño
        fuente = ui->cb_font->currentFont().family() + " " + QVariant(ui->spin_font_tam->value()).toString();
    }
    else{
        fuente = ui->cb_font->currentFont().family() + " " + estilo + " " + QVariant(ui->spin_font_tam->value()).toString();
    }
    appareance->setFont( fuente );



    qDebug() << "******************************************* INSTALLATION :\n"
            << "theme : " << appareance->getTheme() << "\n"
            << "theme path : " << appareance->getThemePath() << "\n"
            << "icons : " << appareance->getIcon() << "\n"
            << "fallback icons : " << appareance->getIconFallBack() << "\n"
            << "font family : " << appareance->getFont() << "\n"
            << "********************************************************";

    //Guardamos configuracion
    if(appareance->saveFileConfig()){

    }
    else{
        QMessageBox::warning(this, "ERROR", "There was not possible to save the config");
    }
    
    
    
}


void Modulo::defaults()
{
     //Establecemos los valores predeterminados de chakra linux
    appareance->setFont("Bitstream Vera Sans 10");
    appareance->setTheme("oxygen-gtk");
    appareance->setThemePath("/usr/share/themes/oxygen-gtk");
    appareance->setIcon("oxygen-refit-2-2.5.0");
    appareance->setIconFallBack("oxygen-refit-2-2.5.0");
    
    //actualizamos la ui
    refreshLists();
}



// ************************ DIALOGS FOR INSTALL AND UNINSTALL THEMES

void Modulo::showDialogForInstall()
{
    
    installer->exec();
    
    //guardamos el tema seleccionado anteriormente
    QString temp;
    
    
    //Refresca la lista de iconos y temas disponibles
    temp = ui->cb_theme->currentText(); //Obtener el preseleccionado
    ui->cb_theme->clear(); //Eliminamos texto
    ui->cb_theme->addItems(appareance->getAvaliableThemes()); //Lo rellenamos de nuevo con los nuevos temas
    ui->cb_theme->setCurrentIndex(ui->cb_theme->findText(temp)); //Establecemos el texto seleccionado anteriormente
    
    //COMBOBOX ICONS
    temp = ui->cb_icon->currentText();
    ui->cb_icon->clear();
    ui->cb_icon->addItems(appareance->getAvaliableIcons());
    ui->cb_icon->setCurrentIndex(ui->cb_icon->findText(temp));
    
    //COMBOBOX ICONS FALLBACK
    temp = ui->cb_icon_fallback->currentText();
    ui->cb_icon_fallback->clear();
    ui->cb_icon_fallback->addItems(appareance->getAvaliableIcons());
    ui->cb_icon_fallback->setCurrentIndex(ui->cb_icon_fallback->findText(temp));
}

void Modulo::showDialogForUninstall()
{
    uninstaller->refresthListsForUninstall();
    uninstaller->exec();
    
     //guardamos el tema seleccionado anteriormente
    QString temp;
    
    
    //Refresca la lista de iconos y temas disponibles
    temp = ui->cb_theme->currentText(); //Obtener el preseleccionado
    ui->cb_theme->clear(); //Eliminamos texto
    ui->cb_theme->addItems(appareance->getAvaliableThemes()); //Lo rellenamos de nuevo con los nuevos temas
    ui->cb_theme->setCurrentIndex(ui->cb_theme->findText(temp)); //Establecemos el texto seleccionado anteriormente
    
    //COMBOBOX ICONS
    temp = ui->cb_icon->currentText();
    ui->cb_icon->clear();
    ui->cb_icon->addItems(appareance->getAvaliableIcons());
    ui->cb_icon->setCurrentIndex(ui->cb_icon->findText(temp));
    
    //COMBOBOX ICONS FALLBACK
    temp = ui->cb_icon_fallback->currentText();
    ui->cb_icon_fallback->clear();
    ui->cb_icon_fallback->addItems(appareance->getAvaliableIcons());
    ui->cb_icon_fallback->setCurrentIndex(ui->cb_icon_fallback->findText(temp));
}


void Modulo::makePreviewFont()
{
    
    //Hacer un preview de las fuentes
    QFont fuente(ui->cb_font->currentFont());
    
    //Investigamos el efecto de la fuente
    bool bold = false, italic = false;
    
    //Dependiendo del indice del combobox es el estilo de la fuente
    int pos = ui->cb_font_style->currentIndex();
    
    if(pos == 1)
        bold = true;
    else if(pos == 2)
        italic = true;
    else if(pos == 3){
        bold = true;
        italic = true;
    }
    else{
        bold = false;
        italic = false;
    }
    
    fuente.setBold(bold);
    fuente.setItalic(italic);
    
    //Establecemos el tamanio de la fuente
    fuente.setPointSize(ui->spin_font_tam->value());
    
    ui->lb_font_preview->setFont(fuente);
    
    
        
}   
