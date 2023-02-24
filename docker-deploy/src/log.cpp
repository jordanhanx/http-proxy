#include <iostream>
#include <fstream>

class Logger{
  public:
  Logger() {
    myFile.open("proxy.log");
    if(!myFile.is_open()){
      std::cerr << "Unable to open file";
    }
  }

  Logger& operator<< (const auto input){
    if (myFile.is_open()) {
      myFile << input;
      myFile.flush();
  }
    return *this;
  }
  
  ~Logger(){
    if (myFile.is_open()) {
      myFile.close();
      myFile.flush();
  }
  }

  private:
  std::ofstream myFile;
};