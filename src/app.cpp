#include "app.h"

app::app(){
  setStyleSheet("\
			QWidget{background:rgb(235, 250, 235);}\
      \
      ");

    /* top widget */{
    layout_top.addWidget(&btn_play);
    btn_play.setText("Play");
    layout_top.addWidget(&btn_load);
    btn_load.setText("Load");

    layout_top.addWidget(new QLabel("mpNC"));
    layout_top.addWidget(&chk_mpNC);
    layout_top.addWidget(new QLabel("Priority"));
    layout_top.addWidget(&chk_priority);

    QObject::connect(&chk_priority, &QCheckBox::stateChanged, [&](int state) {
      if (state == Qt::Checked) {
        proc.EnableRealtimeMode(true);
      }
      else {
        proc.EnableRealtimeMode(false);
      }
      });

    QObject::connect(&chk_mpNC, &QCheckBox::stateChanged, [&](int state) {
      if (state == Qt::Checked) {
        proc.EnablempNC(true);
      }
      else {
        proc.EnablempNC(false);
      }
      });


    // Param Edits
    /*
    https://doc.qt.io/qt-6/qdoublevalidator.html
    QDoubleValidator(double bottom, double top, int decimals, QObject *parent = nullptr)
    */
   
    layout_main.addLayout(&layout_top);
    layout_top.setAlignment(Qt::AlignLeft);

    QObject::connect(&proc, &processor::signal_append_log, &widget_log, &WidgetLog::SlotAppendLog);

    QObject::connect(&btn_play, &QPushButton::pressed, this, &app::slot_btn_play);
    /* Processor */
    QObject::connect(&btn_play, &QPushButton::pressed, &proc, &processor::slot_toggle);

    QObject::connect(&btn_load, &QPushButton::clicked, [&]() {
      QString fileName;
      QFileDialog dialog;

      /* signal - slot for import button */
      fileName = dialog.getOpenFileName(this,
        tr("Open Wav File"), ".", tr("something (*.wav)"));
      if (!fileName.isEmpty()){
        emit(signal_load(fileName));
        setProcParam();
       }
      }
    );
    QObject::connect(this, &app::signal_load, &proc, &processor::slot_load);
  }

  /* main widget */{
    widget_main.setStyleSheet("\
			QWidget{background:rgb(220, 250, 220);}\
      \
      ");

    widget_main.addTab(&widget_log, "log");
    widget_log.setStyleSheet("\
		QWidget{background:rgb(255,255,255);}\
    QPlainTextEdit{background:rgb(255, 255, 255);}\
    \
    ");

    widget_main.addTab(&widget_config, "parameters");
    widget_config.setStyleSheet("\
			QWidget{background:rgb(255,255,255);}\
			QLineEdit{background:rgb(238, 238, 238);}\
			QComboBox{background:rgb(238, 238, 238);}\
      \
      ");

    widget_config.Add("Input/Output", "../config/io.json");
    layout_main.addWidget(&widget_main);
  }

  setMinimumSize(840,640);
  setLayout(&layout_main);

  // Apply param
};

app::~app() {

};

void app::slot_btn_play() {
  // Stop
  if (is_playing) {
    btn_play.setText("Play");
  }
  // Start
  else {
    setProcParam();
    btn_play.setText("Stop");
  }
  is_playing = !is_playing;
  std::cout << is_playing << std::endl;
}

 double app::get(QString a, string b) {
  double val;
   json j = widget_config[a.toStdString()][b]["value"];
   switch (j.type()) {
    //case json::value_t::object :
      //break;
    case json::value_t::boolean :
      return static_cast<double>(j.get<bool>());
    case json::value_t::number_float :
      return j.get<double>();
    case json::value_t::number_unsigned :
      return static_cast<double>(j.get<int>());
    case json::value_t::number_integer :
      return static_cast<double>(j.get<int>());
    default:
      printf("ERROR::unsupported json type:%d\n",j.type());
    }
 }

 void app::setProcParam() {
   proc.device_in = static_cast<int>(get("Input/Output", "input_device"));
   proc.device_out = static_cast<int>(get("Input/Output", "output_device"));
   proc.multiple = static_cast<double>(get("Input/Output", "multiple"));

 }

