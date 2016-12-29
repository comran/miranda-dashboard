package dashboard;

message SensorsQueue {
  double rpm;
  double mph;
  double coolant_temp;
};
queue SensorsQueue sensors_queue;
