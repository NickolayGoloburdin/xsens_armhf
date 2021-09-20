#include "xsensereciever.hpp"
Journaller *gJournal = 0;
int main(void) {
  XsensReciever a;
  a.setConfigs();
  XsensReciever::GnssImu pack;
  a.recieve();
}
