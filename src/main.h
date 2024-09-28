#ifndef MAIN_H
#define MAIN_H

void mDNS_start();

void connectWifi();
// void handleLongBtn();
void handleTmrNetworkOverseer();
void setupCoordinatorMode();
void startAP(const bool start);

// void toggleUsbMode();


void stopWifi();
void checkFileSys();

struct TaskParams
{
  const char *url;
};


#endif // MAIN_H