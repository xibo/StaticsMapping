#include <QJsonDocument>
#include <QJsonParseError>

#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>

#include <QCryptographicHash>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QImageReader>
#include <QKeySequence>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <Common/Classification/Aircraft.hpp>
#include <Common/Classification/Airline.hpp>
#include <Common/Classification/Library.hpp>
#include <Common/Classification/Object.hpp>

#include <Common/Classification/AircraftModel.hpp>
#include <Common/Classification/AirlineModel.hpp>
#include <Common/Classification/CategoryModelDisplay.hpp>
#include <Common/Classification/LibraryModel.hpp>
#include <Common/Classification/ObjectModel.hpp>

#include <Common/Widgets/AircraftDelegate.hpp>
#include <Common/Widgets/AirlineDelegate.hpp>

#include <Common/Widgets/AircraftTableDialog.hpp>
#ifdef HAVE_GRAPHS
#  include <Common/Widgets/AirlineScreen.hpp>
#endif
#include <Common/Widgets/AirlineTableDialog.hpp>
#include <Common/Widgets/LibraryTableDialog.hpp>
#include <Common/Widgets/ObjectTableDialog.hpp>

#include "InsertObjectConfirmationDialog.hpp"
#include "MainWindow.hpp"
#include "VisualObjectsModel.hpp"

#include <Common/Widgets/GlobalDistributionDialog.hpp>



MainWindow::MainWindow (QWidget *parent , Qt::WindowFlags flags)
: QMainWindow (parent, flags)
, m_definitions (Classification::Definitions::fromFile())
, m_obj_screen(new Widgets::ObjScreen())
, m_objects()
, m_object_select(createFormComboBox())
, m_object_data_model (m_definitions->objectModel ())
, m_object_data_mapper (new QDataWidgetMapper(this))
, m_current_visual_object_index (-1)
{
  createWidgets ();
  
  
  setMenuBar(createMenuBar());
  
  //GlobalDistributionDialog *d = new GlobalDistributionDialog(m_definitions);
  //d->show ();
  
  setAcceptDrops (true);
}



MainWindow::~MainWindow()
{
}



void
MainWindow::loadObjFile (QString path)
{
  try {
    Widgets::VisualObjectPointer obj(
      new Widgets::VisualObject(m_definitions, path)
    );
    
    if (m_definitions->object(obj->data->primaryKey())) {
      nonInteractiveInsert(obj);
    } else {
      interactiveInsert(obj);
    }
    
    //m_object_data_mapper->setCurrentIndex (m_objects.size () - 1);
    
  } catch (const Obj8::Parser::SyntaxError &error) {
    
    Obj8::Parser::LexerContext lc = error.info ();
    
    QMessageBox::critical (
      this,
      "Fatal",
      QString("Error at %1:%2.")
        .arg(lc.line())
        .arg(lc.column())
    );
  }
}



void
MainWindow::loadObjFile ()
{
  QString filename;
  
  filename = QFileDialog::getOpenFileName (this, "Open File", QString(),
        "OBJ8 Files (*.obj)", Q_NULLPTR,
        QFileDialog::DontResolveSymlinks);
  
  if (filename.isNull())
    return;
  
  loadObjFile (filename);
}



void
MainWindow::saveObjFile ()
{
  QString filename;
  QSharedPointer<Obj8::File> file;
  
  Widgets::VisualObjectPointer ptr(m_objects.value(m_current_visual_object_index));
  
  if (ptr) {
    file = ptr->file;
  }

  if (file.isNull()) {
    return;
  }
  
  filename = QFileDialog::getSaveFileName(
    this,
    "Save File",
    QString(),
    "OBJ8 Files (*.obj)",
    Q_NULLPTR,
    QFileDialog::DontResolveSymlinks
  );
  
  if (filename.isNull())
    return;
  
  QFile output(filename);
  if (output.open (QFile::WriteOnly | QFile::Truncate)) {
    output.write (file->toString().toUtf8());
  } else {
    QMessageBox::warning(this, "Failure", "Cannot write to file");
  }
  setWindowTitle (filename);
}



void
MainWindow::saveDefinitions ()
{
  m_definitions->toFile();
}



void
MainWindow::saveDefinitionsAs ()
{
  QString filename;
  
  filename = QFileDialog::getSaveFileName (this, "Save File", QString(),
        "JSON Archive (*.json)", Q_NULLPTR,
        QFileDialog::DontResolveSymlinks);
  
  if (filename.isNull())
    return;
  
  saveDefinitionsAs (filename);
}



void
MainWindow::saveDefinitionsAs (QString path)
{
  if (m_definitions) {
    m_definitions->toFile(path);
  }
}



void
MainWindow::unloadObjFile ()
{
  VisualObjectsModel *mdl;
  int idx;
  
  idx = m_current_visual_object_index;
  mdl = dynamic_cast<VisualObjectsModel*>(m_object_select->model());
  
  Q_CHECK_PTR (mdl);
  
  if (m_objects.value(idx)) {
    m_objects.removeAt(idx);
    mdl->reset();
    m_object_select->setCurrentIndex(-1);
  }
}



void
MainWindow::showAircraftTableDialog ()
{
  Widgets::AircraftTableDialog(m_definitions, this).exec();
}



void
MainWindow::showAirlineHierarchyDialog ()
{
#ifdef HAVE_GRAPHS
  Widgets::AirlineScreen(m_definitions.data(), this).exec();
#endif
}



void
MainWindow::showAirlineTableDialog ()
{
  Widgets::AirlineTableDialog(m_definitions, this).exec();
}



void
MainWindow::showLibraryTableDialog ()
{
  Widgets::LibraryTableDialog(m_definitions, this).exec();
}



void
MainWindow::showObjectTableDialog ()
{
  Widgets::ObjectTableDialog(m_definitions, this).exec();
}



void
MainWindow::showSupportedImageFormats ()
{
  QStringList formats;
  QList<QByteArray> mime_types = QImageReader::supportedMimeTypes();
  
  Q_FOREACH (QByteArray ba, mime_types) {
    formats += QString::fromUtf8 (ba);
  }
  
  QMessageBox::information(this, "Supported Image Formats", formats.join("\n"));
}



void
MainWindow::nonInteractiveInsert(Widgets::VisualObjectPointer obj)
{
  m_objects.append(obj);
  m_definitions->upsert(obj->data);
}



void
MainWindow::interactiveInsert(Widgets::VisualObjectPointer obj)
{
  InsertObjectConfirmationDialog dialog(obj, this);
  if (dialog.exec() == QDialog::Accepted) {
    nonInteractiveInsert(obj);
  }
}



void
MainWindow::dragEnterEvent (QDragEnterEvent *event)
{
  event->acceptProposedAction();
}



void
MainWindow::dropEvent (QDropEvent *event)
{
  QString data, uri, path;
  QUrl    url;
  
  data = event->mimeData()->text ();
  
  Q_FOREACH (QString file, data.split("\n")) {
    
    uri  = file;
    
    url  = QUrl::fromUserInput (uri);
    path = url.toLocalFile();
    
    if (QFile::exists (path)) {
      loadObjFile (path);
    } else {
      qWarning ("Could not process %s", qUtf8Printable(uri));
    }
  }
}



void
MainWindow::setDisplayedVisualObject (int visual_object_index)
{
  if (visual_object_index == -1) {
    if ((0 <= m_current_visual_object_index) and (m_current_visual_object_index < m_objects.size())) {
      m_object_select->setCurrentIndex (m_current_visual_object_index);
      return;
    }
  }
  
  m_current_visual_object_index = visual_object_index;
  
  Widgets::VisualObjectPointer ptr(m_objects.value(visual_object_index));
  QSharedPointer<Widgets::VisualModel> mdl;
  Classification::ObjectPointer obj;
  
  if (ptr) {
    obj = ptr->data;
    mdl = ptr->model;
    
    int metadata_index = m_definitions->indexOf(obj);
    m_object_data_mapper->setCurrentIndex(metadata_index);
  }
  
  m_obj_screen->setModel(obj, mdl);
}



QMenuBar *
MainWindow::createMenuBar()
{
  QMenuBar *menubar(new QMenuBar());
  
  QMenu *obj8(menubar->addMenu("Object"));
  obj8->addAction("Open OBJ8...", this, SLOT(loadObjFile()));
  obj8->addAction("Save OBJ8...", this, SLOT(saveObjFile()));
  obj8->addSeparator();
  obj8->addAction("Exit", this, SLOT(close()));
  
  QMenu *defs(menubar->addMenu("Definitions"));
  defs->addSeparator();
  defs->addAction("Save Definitions", this, SLOT(saveDefinitions()));
  defs->addAction("Save Definitions As...", this, SLOT(saveDefinitionsAs()));
  defs->addSeparator();
  defs->addAction("Aircrafts Table", this, SLOT(showAircraftTableDialog()));
  defs->addAction("Airline Hierarchies", this, SLOT(showAirlineHierarchyDialog()));
  defs->addAction("Airline Table", this, SLOT(showAirlineTableDialog()));
  defs->addAction("Library Table", this, SLOT(showLibraryTableDialog()));
  defs->addAction("Object Table", this, SLOT(showObjectTableDialog()));
  
  QMenu *help(menubar->addMenu("Help"));
  help->addAction("Supported Image Formats", this, SLOT(showSupportedImageFormats()));
  
  return (menubar);
}


QComboBox *
MainWindow::createFormComboBox(QStringList content)
{
  QComboBox *combobox(new QComboBox());
  combobox->setSizeAdjustPolicy (QComboBox::AdjustToMinimumContentsLength);
  combobox->setMinimumContentsLength(32);
  combobox->addItems(content);
  return (combobox);
}



QComboBox *
MainWindow::createFormComboBox(QMetaEnum e)
{
  QStringList content;
  for (int i=0; i!=e.keyCount(); ++i) {
    content.append(QString::fromUtf8(e.key(i)));
  }
  return (createFormComboBox(content));
}



QDoubleSpinBox
*MainWindow::createFormMetricInputSpinner()
{
  QDoubleSpinBox *spinbox(new QDoubleSpinBox());
  spinbox->setSingleStep(0.1);
  spinbox->setRange(-100, 100);
  spinbox->setDecimals(3);
  return (spinbox);
}



QLayout *
MainWindow::createForm()
{
  QDoubleSpinBox *locale[3];
  
  QBoxLayout *form_container(new QVBoxLayout());
  QFormLayout *form(new QFormLayout());
  
  QComboBox *aircraft(createFormComboBox());
  QComboBox *livery(createFormComboBox());
  QComboBox *library(createFormComboBox());
  QComboBox *usage(createFormComboBox(QMetaEnum::fromType<Classification::Object::Purpose>()));
  QComboBox *fictive(createFormComboBox(QStringList() << "true" << "false"));
  QComboBox *rotate(createFormComboBox(QMetaEnum::fromType<Classification::Object::Rotate>()));
  QSpinBox  *introduced(new QSpinBox());
  QSpinBox  *retired(new QSpinBox());
  QLineEdit *filehash(new QLineEdit());
  QLineEdit *filesize(new QLineEdit());
  QLineEdit *filename(new QLineEdit());
  QTextEdit *comment(new QTextEdit());
  QPushButton *purge(new QPushButton());
  
  VisualObjectsModel *visual_objects_model = new VisualObjectsModel (&m_objects);
  
  m_object_select->setModel (visual_objects_model);
  
  filehash->setEnabled (false);
  filesize->setEnabled (false);
  
  introduced->setRange (0, 2100);
  retired->setRange (0, 2100);

  form->addRow ("Displayed", m_object_select);
  form->addRow ("Filesize", filesize);
  form->addRow ("SHA-1", filehash);
  form->addRow ("Filename", filename);
  form->addRow ("Aircraft", aircraft);
  form->addRow ("Livery", livery);
  form->addRow ("Usage", usage);
  form->addRow ("Library", library);
  form->addRow ("Comment", comment);
  form->addRow ("Fictive", fictive);
  form->addRow ("Introduction", introduced);
  form->addRow ("Retirement", retired);
  form->addRow ("Rotation", rotate);
  
  for (size_t i=0; i!=nitems(locale); ++i) {
    locale[i] = createFormMetricInputSpinner();
    
    connect (locale[i], SIGNAL(valueChanged(QString)), m_obj_screen, SLOT(update()));
    
    form->addRow (QString::asprintf("Translation (%c)", int('X' + i)), locale[i]);
  }
  
  Classification::AircraftModel *acfmdl = m_definitions->aircraftModel ();
  Classification::AirlineModel  *almdl = m_definitions->airlineModel ();
  Classification::LibraryModel  *libmdl = m_definitions->libraryModel ();
  aircraft->setModel (acfmdl);
  aircraft->setModelColumn (acfmdl->column(Classification::Aircraft::IcaoProperty));
  livery->setModel (almdl);
  livery->setModelColumn (almdl->column(Classification::Airline::IcaoProperty));
  library->setModel (libmdl);
  library->setModelColumn (libmdl->column(Classification::Library::KeyProperty));
  
  aircraft->setItemDelegate (new Widgets::AircraftDelegate(m_definitions));
  livery->setItemDelegate (new Widgets::AirlineDelegate(m_definitions));
  
  m_object_data_mapper->setModel (m_object_data_model);
  m_object_data_mapper->addMapping (filesize, m_object_data_model->column(Classification::Object::FilesizeProperty));
  //m_object_data_mapper->addMapping (filehash, m_object_data_model->column(Classification::Object::FilehashProperty));
  m_object_data_mapper->addMapping (filename, m_object_data_model->column(Classification::Object::FilenameProperty));
  m_object_data_mapper->addMapping (fictive, m_object_data_model->column(Classification::Object::FictiveProperty), "currentText");
  m_object_data_mapper->addMapping (aircraft, m_object_data_model->column(Classification::Object::AircraftProperty), "currentText");
  m_object_data_mapper->addMapping (livery, m_object_data_model->column(Classification::Object::LiveryProperty), "currentText");
  m_object_data_mapper->addMapping (library, m_object_data_model->column(Classification::Object::LibraryProperty), "currentText");
  m_object_data_mapper->addMapping (usage, m_object_data_model->column(Classification::Object::PurposeProperty), "currentText");
  
  m_object_data_mapper->addMapping (comment, m_object_data_model->column(Classification::Object::CommentProperty), "plainText");
  m_object_data_mapper->addMapping (introduced, m_object_data_model->column(Classification::Object::IntroductionProperty));
  m_object_data_mapper->addMapping (retired, m_object_data_model->column(Classification::Object::RetirementProperty));
  
  m_object_data_mapper->addMapping (rotate, m_object_data_model->column(Classification::Object::RotateProperty));
  m_object_data_mapper->addMapping (locale[0], m_object_data_model->column(Classification::Object::TranslationXProperty));
  m_object_data_mapper->addMapping (locale[1], m_object_data_model->column(Classification::Object::TranslationYProperty));
  m_object_data_mapper->addMapping (locale[2], m_object_data_model->column(Classification::Object::TranslationZProperty));
  m_object_data_mapper->setSubmitPolicy (QDataWidgetMapper::AutoSubmit);
  
  connect (
      m_object_select, SIGNAL(currentIndexChanged(int)),
      this, SLOT(setDisplayedVisualObject(int))
  );
  
  connect (
      m_object_data_model, &Classification::ObjectModel::modelReset,
      visual_objects_model, &VisualObjectsModel::reset
  );
  connect (
      m_object_data_model, &Classification::ObjectModel::dataChanged,
      visual_objects_model, &VisualObjectsModel::reset
  );
  
  purge->setText ("close");
  connect (
      purge, &QPushButton::released,
      this, &MainWindow::unloadObjFile
  );
  
  form_container->addItem (form);
  form_container->addStretch();
  form_container->addWidget (purge);
  
  return (form_container);
}



void
MainWindow::createWidgets ()
{
  QWidget *dummy(new QWidget());
  QLayout *form(createForm());
  
  QHBoxLayout *ext(new QHBoxLayout());
  ext->addLayout(form);
  ext->addWidget(m_obj_screen);
  dummy->setLayout(ext);
  
  setCentralWidget (dummy);
}
