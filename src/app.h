#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QComboBox>
#include <QToolButton>
#include <QFileDialog>
#include <QDateTime >
#include <QLineEdit>

#include <QCheckBox>

#include <QMessageBox>

#include "WidgetConfig.h"
#include "WidgetLog.h"
#include "processor.h"

class app : public QWidget {
  Q_OBJECT

private:
  QVBoxLayout layout_main;
  QHBoxLayout layout_top;
  QPushButton btn_play;
  QPushButton btn_load;

  QCheckBox chk_mpNC;
  QCheckBox chk_priority;

  QTabWidget widget_main;
  WidgetConfig widget_config;
  WidgetLog widget_log;

  bool is_playing = false;

  //template <typename T>
  //T get(QString, string);
  double get(QString, string);

  void setProcParam();

  processor proc;

public:

  app();
  ~app();

public slots:
  void slot_btn_play();


signals:
  void signal_process();
  void signal_load(QString fileName);
};
