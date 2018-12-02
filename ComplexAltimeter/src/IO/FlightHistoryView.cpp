#include "FlightHistoryView.hpp"
#include "../DataLogger.hpp"

void FlightHistoryView::setHistoryInfo(String info) { 
    setText(info, 0, false); 
    update();    
}