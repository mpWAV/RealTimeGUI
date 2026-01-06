#include "WidgetConfig.h"

WidgetConfig::WidgetConfig() {
  setLayout(&layout_main);

  layout_main.addLayout(&layout_audio);

  btn_audioprobe.setText("AudioProbe");
  layout_audio.addWidget(&btn_audioprobe);
  layout_audio.addWidget(&TB_audio);

  layout_main.addWidget(&widget_config);

  QObject::connect(&btn_audioprobe, &QPushButton::clicked, this, &WidgetConfig::slot_audioprobe);

  slot_audioprobe();
}

WidgetConfig::~WidgetConfig() {

}


void WidgetConfig::slot_audioprobe() {

  map_device.clear();


  text_device = " *** Device List *** \n\n";
 
  RtAudio audio;
  RtAudio::DeviceInfo info;

  std::cout << "\nAPI: " << RtAudio::getApiDisplayName(audio.getCurrentApi()) << std::endl;

  std::vector<unsigned int> devices = audio.getDeviceIds();
  std::cout << "\nFound " << devices.size() << " device(s) ...\n";

  for (unsigned int i = 0; i < devices.size(); i++) {
    info = audio.getDeviceInfo(devices[i]);
    QString temp_device = "[";
    //temp_device.append(QString::fromStdString(to_string(i)));
    //temp_device.append(QString::number(i));
    temp_device.append(QString::number(info.ID));
    temp_device.append("]");
    // QString name_dev = QString::fromLocal8Bit(info.name.c_str());
     //QString name_dev = QString::fromUtf8(info.name.c_str());
    temp_device.append(QString::fromStdString(info.name));
    // temp_device.append(name_dev);
    // std::cout<<"device name " << info.name<<"\n";
    map_device.insert(make_pair(temp_device.toStdString(), i));

    text_device.append(temp_device);

    text_device.append("\n");
    /*
    if (info.probed == false) {
      text_device.append("Probe Status = Unsuccessful");
      text_device.append("\n");
    }
    else {
    */
      //  std::cout << "Probe Status = Successful\n";
      text_device.append("Output Channels = ");
      text_device.append(
        QString::fromStdString(to_string(info.outputChannels)));
      text_device.append("\nInput Channels = ");
      text_device.append(
        QString::fromStdString(to_string(info.inputChannels)));

      text_device.append("\n");
      if (info.sampleRates.size() < 1) {
        text_device.append("No supported sample rates found!");
      }
      else {
        text_device.append("Supported sample rates = ");
        for (unsigned int j = 0; j < info.sampleRates.size(); j++) {
          text_device.append(QString::fromStdString(
            to_string(info.sampleRates[j])
          ));
          text_device.append(" ");
        }
      }
      text_device.append("\n");
   // }
    text_device.append("\n");
  }
  text_device.append("\n");

  TB_audio.setText(text_device);
}

