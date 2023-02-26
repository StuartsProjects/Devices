/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 22/12/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
  /*******************************************************************************************************/


//locations in FRAM for saving local key and session variables
const uint16_t local_artKey_addr = 16;           //16 x uint8_t array
const uint16_t local_nwkKey_addr = 32;           //16 x uint8_t array
const uint16_t local_channelFreq_base = 80;      //uint32_t 16 channels

const uint16_t  local_rssi_addr = 256;           // int8_t
const uint16_t  local_snr_addr  = 257;           // int8_t
const uint16_t  local_txpow_addr  = 258;         // int8_t
const uint16_t  local_adrTxPow_addr = 259;       // int8_t
const uint16_t  local_adrAckReq_addr  = 260;     // int8_t
const uint16_t  local_rxsyms_addr = 261;         // uint8_t
const uint16_t  local_dndr_addr = 262;           // uint8_t
const uint16_t  local_txChnl_addr = 263;         // uint8_t
const uint16_t  local_globalDutyRate_addr = 264; // uint8_t
const uint16_t  local_upRepeat_addr = 265;       // uint8_t
const uint16_t  local_datarate_addr = 266;       // uint8_t
const uint16_t  local_errcr_addr  = 267;         // uint8_t
const uint16_t  local_rejoinCnt_addr  = 268;     // uint8_t
const uint16_t  local_pendTxPort_addr = 269;     // uint8_t
const uint16_t  local_pendTxConf_addr = 270;     // uint8_t
const uint16_t  local_pendTxLen_addr  = 271;     // uint8_t
const uint16_t  local_dnConf_addr = 275;         // uint8_t
const uint16_t  local_adrChanged_addr = 276;     // uint8_t
const uint16_t  local_rxDelay_addr  = 277;       // uint8_t
const uint16_t  local_margin_addr = 278;         // uint8_t
const uint16_t  local_ladrAns_addr  = 279;       // uint8_t
const uint16_t  local_devsAns_addr  = 280;       // uint8_t
const uint16_t  local_adrEnabled_addr = 281;     // uint8_t
const uint16_t  local_moreData_addr = 282;       // uint8_t
const uint16_t  local_dutyCapAns_addr = 283;     // uint8_t
const uint16_t  local_snchAns_addr  = 284;       // uint8_t
const uint16_t  local_dn2Dr_addr  = 285;         // uint8_t
const uint16_t  local_dn2Ans_addr = 286;         // uint8_t
const uint16_t  local_missedBcns_addr = 287;     // uint8_t
const uint16_t  local_bcninfoTries_addr = 288;   // uint8_t
const uint16_t  local_pingSetAns_addr = 289;     // uint8_t
const uint16_t  local_txCnt_addr  = 290;         // uint8_t
const uint16_t  local_txrxFlags_addr  = 291;     // uint8_t
const uint16_t  local_dataBeg_addr  = 292;       // uint8_t
const uint16_t  local_dataLen_addr  = 293;       // uint8_t
const uint16_t  local_bcnChnl_addr  = 295;       // uint8_t
const uint16_t  local_bcnRxsyms_addr  = 296;     // uint8_t

const uint16_t  local_drift_addr  = 352;         // int16_t
const uint16_t  local_lastDriftDiff_addr  = 354; // int16_t
const uint16_t  local_maxDriftDiff_addr = 356;   // int16_t
const uint16_t  local_rps_addr  = 358;           // uint16_t
const uint16_t  local_channelMap_addr = 362;     // uint16_t
const uint16_t  local_opmode_addr = 364;         // uint16_t
const uint16_t  local_clockError_addr = 366;     // uint16_t
const uint16_t  local_devNonce_addr = 368;       // uint16_t

const uint16_t  local_txend_addr  = 416;         // int32_t
const uint16_t  local_rxtime_addr = 420;         // int32_t
const uint16_t  local_globalDutyAvail_addr  = 424; // int32_t
const uint16_t  local_bcnRxtime_addr  = 428;     // int32_t
const uint16_t  local_freq_addr = 432;           // uint32_t
const uint16_t  local_netid_addr  = 440;         // uint32_t
const uint16_t  local_devaddr_addr  = 444;       // uint32_t
const uint16_t  local_seqnoDn_addr  = 448;       // uint32_t
const uint16_t  local_seqnoUp_addr  = 452;       // uint32_t
const uint16_t  local_dn2Freq_addr  = 456;       // uint32_t
const uint16_t  local_sentpayloads_addr = 460;   // uint32_t
const uint16_t  local_FUPpayloads_addr = 464;    // uint32_t

void print_fixed_session_data();
void print_variable_session_data();
void print_channels();

void save_fixed_session_data();
void save_variable_session_data();
void save_channels();

void print_fixed_session_data_FRAM();
void print_variable_session_data_FRAM();
void print_channels_FRAM();

void restore_fixed_session_data();
void restore_variable_session_data();
void restore_channels();


void print_fixed_session_data()
{
  MonitorPort.print(F("artKey: "));
  for (int i = 0; i < sizeof(LMIC.artKey); i++) {
    if (i != 0)
      MonitorPort.print(F("-"));
    if (LMIC.artKey[i] < 16)
      MonitorPort.print(F("0"));
    MonitorPort.print(LMIC.artKey[i], HEX);
  }
  MonitorPort.println();

  MonitorPort.print(F("nwkKey: "));
  for (int i = 0; i < sizeof(LMIC.nwkKey); i++) {
    if (i != 0)
      MonitorPort.print(F("-"));
    if (LMIC.nwkKey[i] < 16)
      MonitorPort.print(F("0"));
    MonitorPort.print(LMIC.nwkKey[i], HEX);
  }
  MonitorPort.println();

  MonitorPort.print(F("devNonce: 0x"));
  MonitorPort.println(LMIC.devNonce, HEX);

  MonitorPort.print(F("netid: 0x"));
  MonitorPort.println(LMIC.netid, HEX);

  MonitorPort.print(F("devaddr: 0x"));
  MonitorPort.println(LMIC.devaddr, HEX);

}

void print_variable_session_data()
{
MonitorPort.print(F("rssi: 0x"));
MonitorPort.println(LMIC.rssi, HEX);

MonitorPort.print(F("snr: 0x"));
MonitorPort.println(LMIC.snr, HEX);

MonitorPort.print(F("txpow: "));
MonitorPort.println(LMIC.txpow, DEC);

MonitorPort.print(F("adrTxPow: "));
MonitorPort.println(LMIC.adrTxPow, DEC);

MonitorPort.print(F("adrAckReq: 0x"));
MonitorPort.println(LMIC.adrAckReq, HEX);

MonitorPort.print(F("rxsyms: 0x"));
MonitorPort.println(LMIC.rxsyms, HEX);

MonitorPort.print(F("dndr: 0x"));
MonitorPort.println(LMIC.dndr, HEX);

MonitorPort.print(F("txChnl: 0x"));
MonitorPort.println(LMIC.txChnl, HEX);

MonitorPort.print(F("globalDutyRate: 0x"));
MonitorPort.println(LMIC.globalDutyRate, HEX);

MonitorPort.print(F("upRepeat: 0x"));
MonitorPort.println(LMIC.upRepeat, HEX);

MonitorPort.print(F("datarate: 0x"));
MonitorPort.println(LMIC.datarate, HEX);

MonitorPort.print(F("errcr: 0x"));
MonitorPort.println(LMIC.errcr, HEX);

MonitorPort.print(F("rejoinCnt: 0x"));
MonitorPort.println(LMIC.rejoinCnt, HEX);

MonitorPort.print(F("pendTxPort: 0x"));
MonitorPort.println(LMIC.pendTxPort, HEX);

MonitorPort.print(F("pendTxConf: 0x"));
MonitorPort.println(LMIC.pendTxConf, HEX);

MonitorPort.print(F("pendTxLen: 0x"));
MonitorPort.println(LMIC.pendTxLen, HEX);

MonitorPort.print(F("dnConf: 0x"));
MonitorPort.println(LMIC.dnConf, HEX);

MonitorPort.print(F("adrChanged: 0x"));
MonitorPort.println(LMIC.adrChanged, HEX);

MonitorPort.print(F("rxDelay: 0x"));
MonitorPort.println(LMIC.rxDelay, HEX);

MonitorPort.print(F("margin: 0x"));
MonitorPort.println(LMIC.margin, HEX);

MonitorPort.print(F("ladrAns: 0x"));
MonitorPort.println(LMIC.ladrAns, HEX);

MonitorPort.print(F("devsAns: 0x"));
MonitorPort.println(LMIC.devsAns, HEX);

MonitorPort.print(F("adrEnabled: 0x"));
MonitorPort.println(LMIC.adrEnabled, HEX);

MonitorPort.print(F("moreData: 0x"));
MonitorPort.println(LMIC.moreData, HEX);

MonitorPort.print(F("dutyCapAns: 0x"));
MonitorPort.println(LMIC.dutyCapAns, HEX);

MonitorPort.print(F("snchAns: 0x"));
MonitorPort.println(LMIC.snchAns, HEX);

MonitorPort.print(F("dn2Dr: 0x"));
MonitorPort.println(LMIC.dn2Dr, HEX);

MonitorPort.print(F("dn2Ans: 0x"));
MonitorPort.println(LMIC.dn2Ans, HEX);

MonitorPort.print(F("missedBcns: 0x"));
MonitorPort.println(LMIC.missedBcns, HEX);

MonitorPort.print(F("bcninfoTries: 0x"));
MonitorPort.println(LMIC.bcninfoTries, HEX);

MonitorPort.print(F("pingSetAns: 0x"));
MonitorPort.println(LMIC.pingSetAns, HEX);

MonitorPort.print(F("txCnt: 0x"));
MonitorPort.println(LMIC.txCnt, HEX);

MonitorPort.print(F("txrxFlags: 0x"));
MonitorPort.println(LMIC.txrxFlags, HEX);

MonitorPort.print(F("dataBeg: 0x"));
MonitorPort.println(LMIC.dataBeg, HEX);

MonitorPort.print(F("dataLen: 0x"));
MonitorPort.println(LMIC.dataLen, HEX);

MonitorPort.print(F("bcnChnl: 0x"));
MonitorPort.println(LMIC.bcnChnl, HEX);

MonitorPort.print(F("bcnRxsyms: 0x"));
MonitorPort.println(LMIC.bcnRxsyms, HEX);

MonitorPort.print(F("drift: 0x"));
MonitorPort.println(LMIC.drift, HEX);

MonitorPort.print(F("lastDriftDiff: 0x"));
MonitorPort.println(LMIC.lastDriftDiff, HEX);

MonitorPort.print(F("maxDriftDiff: 0x"));
MonitorPort.println(LMIC.maxDriftDiff, HEX);

MonitorPort.print(F("rps: 0x"));
MonitorPort.println(LMIC.rps, HEX);

MonitorPort.print(F("channelMap: 0x"));
MonitorPort.println(LMIC.channelMap, HEX);

MonitorPort.print(F("opmode: 0x"));
MonitorPort.println(LMIC.opmode, HEX);

MonitorPort.print(F("clockError: 0x"));
MonitorPort.println(LMIC.clockError, HEX);

//MonitorPort.print(F("devNonce: 0x"));
//MonitorPort.println(LMIC.devNonce, HEX);

MonitorPort.print(F("txend: 0x"));
MonitorPort.println(LMIC.txend, HEX);

MonitorPort.print(F("rxtime: 0x"));
MonitorPort.println(LMIC.rxtime, HEX);

MonitorPort.print(F("globalDutyAvail: "));
MonitorPort.println(LMIC.globalDutyAvail, DEC);

MonitorPort.print(F("bcnRxtime: 0x"));
MonitorPort.println(LMIC.bcnRxtime, HEX);

MonitorPort.print(F("freq: "));
MonitorPort.println(LMIC.freq, DEC);

//MonitorPort.print(F("netid: 0x"));
//MonitorPort.println(LMIC.netid, HEX);

//MonitorPort.print(F("devaddr: 0x"));
//MonitorPort.println(LMIC.devaddr, HEX);

MonitorPort.print(F("seqnoDn: "));
MonitorPort.println(LMIC.seqnoDn, DEC);

MonitorPort.print(F("seqnoUp: "));
MonitorPort.println(LMIC.seqnoUp, DEC);

MonitorPort.print(F("dn2Freq: "));
MonitorPort.println(LMIC.dn2Freq, DEC);

MonitorPort.print(F("sentpayloads: "));
MonitorPort.println(sentpayloads, DEC);

MonitorPort.print(F("FUPpayloads: "));
MonitorPort.println(FUPpayloads, DEC);
}


void print_channels()
{
  uint8_t index;
  MonitorPort.println();
  for (index = 0; index <= (MAX_CHANNELS - 1); index++)
  {
    MonitorPort.print(F("Channel_"));
    MonitorPort.print(index);
    MonitorPort.print(F(": "));
    MonitorPort.println(LMIC.channelFreq[index]);
  }
  MonitorPort.println();
}


void save_fixed_session_data()
{
  uint8_t index;
  for (index = 0; index <= sizeof(LMIC.artKey) - 1; index++)
  {
    writeMemoryUint8(local_artKey_addr + index, LMIC.artKey[index]);
  }

  for (index = 0; index <= sizeof(LMIC.nwkKey) - 1; index++)
  {
    writeMemoryUint8(local_nwkKey_addr + index, LMIC.nwkKey[index]);
  }

  writeMemoryUint32(local_devaddr_addr, LMIC.devaddr);
  writeMemoryUint16(local_devNonce_addr, LMIC.devNonce);
  writeMemoryUint32(local_netid_addr, LMIC.netid);
}


void save_variable_session_data()
{
writeMemoryInt8(local_rssi_addr,LMIC.rssi);
writeMemoryInt8(local_snr_addr,LMIC.snr);
writeMemoryInt8(local_txpow_addr,LMIC.txpow);
writeMemoryInt8(local_adrTxPow_addr,LMIC.adrTxPow);
writeMemoryInt8(local_adrAckReq_addr,LMIC.adrAckReq);
writeMemoryUint8(local_rxsyms_addr,LMIC.rxsyms);
writeMemoryUint8(local_dndr_addr,LMIC.dndr);
writeMemoryUint8(local_txChnl_addr,LMIC.txChnl);
writeMemoryInt8(local_globalDutyRate_addr,LMIC.globalDutyRate);
writeMemoryUint8(local_upRepeat_addr,LMIC.upRepeat);
writeMemoryUint8(local_datarate_addr,LMIC.datarate);
writeMemoryUint8(local_errcr_addr,LMIC.errcr);
writeMemoryUint8(local_rejoinCnt_addr,LMIC.rejoinCnt);
writeMemoryUint8(local_pendTxPort_addr,LMIC.pendTxPort);
writeMemoryUint8(local_pendTxConf_addr,LMIC.pendTxConf);
writeMemoryUint8(local_pendTxLen_addr,LMIC.pendTxLen);
writeMemoryUint8(local_dnConf_addr,LMIC.dnConf);
writeMemoryUint8(local_adrChanged_addr,LMIC.adrChanged);
writeMemoryUint8(local_rxDelay_addr,LMIC.rxDelay);
writeMemoryUint8(local_margin_addr,LMIC.margin);
writeMemoryUint8(local_ladrAns_addr,LMIC.ladrAns);
writeMemoryUint8(local_devsAns_addr,LMIC.devsAns);
writeMemoryUint8(local_adrEnabled_addr,LMIC.adrEnabled);
writeMemoryUint8(local_moreData_addr,LMIC.moreData);
writeMemoryUint8(local_dutyCapAns_addr,LMIC.dutyCapAns);
writeMemoryUint8(local_snchAns_addr,LMIC.snchAns);
writeMemoryUint8(local_dn2Dr_addr,LMIC.dn2Dr);
writeMemoryUint8(local_dn2Ans_addr,LMIC.dn2Ans);
writeMemoryUint8(local_missedBcns_addr,LMIC.missedBcns);
writeMemoryUint8(local_bcninfoTries_addr,LMIC.bcninfoTries);
writeMemoryUint8(local_pingSetAns_addr,LMIC.pingSetAns);
writeMemoryUint8(local_txCnt_addr,LMIC.txCnt);
writeMemoryUint8(local_txrxFlags_addr,LMIC.txrxFlags);
writeMemoryUint8(local_dataBeg_addr,LMIC.dataBeg);
writeMemoryUint8(local_dataLen_addr,LMIC.dataLen);
writeMemoryUint8(local_bcnChnl_addr,LMIC.bcnChnl);
writeMemoryUint8(local_bcnRxsyms_addr,LMIC.bcnRxsyms);
writeMemoryInt16(local_drift_addr,LMIC.drift);
writeMemoryInt16(local_lastDriftDiff_addr,LMIC.lastDriftDiff);
writeMemoryInt16(local_maxDriftDiff_addr,LMIC.maxDriftDiff);
writeMemoryUint16(local_rps_addr,LMIC.rps);
writeMemoryUint16(local_channelMap_addr,LMIC.channelMap);
writeMemoryUint16(local_opmode_addr,LMIC.opmode);
writeMemoryUint16(local_clockError_addr,LMIC.clockError);
writeMemoryUint16(local_devNonce_addr,LMIC.devNonce);
writeMemoryInt32(local_txend_addr,LMIC.txend);
writeMemoryInt32(local_rxtime_addr,LMIC.rxtime);
writeMemoryInt32(local_globalDutyAvail_addr,LMIC.globalDutyAvail);
writeMemoryInt32(local_bcnRxtime_addr,LMIC.bcnRxtime);
writeMemoryUint32(local_freq_addr,LMIC.freq);
writeMemoryUint32(local_netid_addr,LMIC.netid);
writeMemoryUint32(local_devaddr_addr,LMIC.devaddr);
writeMemoryUint32(local_seqnoDn_addr,LMIC.seqnoDn);
writeMemoryUint32(local_seqnoUp_addr,LMIC.seqnoUp);
writeMemoryUint32(local_dn2Freq_addr,LMIC.dn2Freq);
writeMemoryUint32(local_sentpayloads_addr,sentpayloads);
writeMemoryUint32(local_FUPpayloads_addr,FUPpayloads);
}


void save_channels()
{
  uint8_t index;

  for (index = 0; index <= (MAX_CHANNELS - 1); index++)
  {
    writeMemoryUint32((local_channelFreq_base + (index * 4)), LMIC.channelFreq[index]);
  }
}


void print_fixed_session_data_FRAM()
{
  uint8_t index, tempvar;
  MonitorPort.println(F("Fixed session data stored in FRAM"));

  MonitorPort.print(F("artKey: "));
  for (index = 0; index <= sizeof(LMIC.artKey) - 1; index++)
  {
    tempvar = readMemoryUint8(local_artKey_addr + index);
    if (index != 0) MonitorPort.print(F("-"));
    if (tempvar < 16)
    {
      MonitorPort.print(F("0"));
    }
    MonitorPort.print(tempvar, HEX);
  }
  MonitorPort.println();

  MonitorPort.print(F("nwkKey: "));
  for (index = 0; index <= sizeof(LMIC.nwkKey) - 1; index++)
  {
    tempvar = readMemoryUint8(local_nwkKey_addr + index);
    if (index != 0) MonitorPort.print(F("-"));
    if (tempvar < 16)
    {
      MonitorPort.print(F("0"));
    }
    MonitorPort.print(tempvar, HEX);
  }
  MonitorPort.println();

  MonitorPort.print(F("devaddr: 0x"));
  MonitorPort.println(readMemoryUint32(local_devaddr_addr), HEX);

  MonitorPort.print(F("devNonce: 0x"));
  MonitorPort.println(readMemoryUint16(local_devNonce_addr), HEX);

  MonitorPort.print(F("netid: 0x"));
  MonitorPort.println(readMemoryUint32(local_netid_addr));
}


void print_variable_session_data_FRAM()
{
MonitorPort.print(F("rssi: 0x"));
MonitorPort.println(readMemoryInt8(local_rssi_addr), HEX);

MonitorPort.print(F("snr: 0x"));
MonitorPort.println(readMemoryInt8(local_snr_addr), HEX);

MonitorPort.print(F("txpow: "));
MonitorPort.println(readMemoryInt8(local_txpow_addr), DEC);

MonitorPort.print(F("adrTxPow: "));
MonitorPort.println(readMemoryInt8(local_adrTxPow_addr), DEC);

MonitorPort.print(F("adrAckReq: 0x"));
MonitorPort.println(readMemoryInt8(local_adrAckReq_addr), HEX);

MonitorPort.print(F("rxsyms: 0x"));
MonitorPort.println(readMemoryUint8(local_rxsyms_addr), HEX);

MonitorPort.print(F("dndr: 0x"));
MonitorPort.println(readMemoryUint8(local_dndr_addr), HEX);

MonitorPort.print(F("txChnl: 0x"));
MonitorPort.println(readMemoryUint8(local_txChnl_addr), HEX);

MonitorPort.print(F("globalDutyRate: 0x"));
MonitorPort.println(readMemoryInt8(local_globalDutyRate_addr), HEX);

MonitorPort.print(F("upRepeat: 0x"));
MonitorPort.println(readMemoryUint8(local_upRepeat_addr), HEX);

MonitorPort.print(F("datarate: 0x"));
MonitorPort.println(readMemoryUint8(local_datarate_addr), HEX);

MonitorPort.print(F("errcr: 0x"));
MonitorPort.println(readMemoryUint8(local_errcr_addr), HEX);

MonitorPort.print(F("rejoinCnt: 0x"));
MonitorPort.println(readMemoryUint8(local_rejoinCnt_addr), HEX);

MonitorPort.print(F("pendTxPort: 0x"));
MonitorPort.println(readMemoryUint8(local_pendTxPort_addr), HEX);

MonitorPort.print(F("pendTxConf: 0x"));
MonitorPort.println(readMemoryUint8(local_pendTxConf_addr), HEX);

MonitorPort.print(F("pendTxLen: 0x"));
MonitorPort.println(readMemoryUint8(local_pendTxLen_addr), HEX);

MonitorPort.print(F("dnConf: 0x"));
MonitorPort.println(readMemoryUint8(local_dnConf_addr), HEX);

MonitorPort.print(F("adrChanged: 0x"));
MonitorPort.println(readMemoryUint8(local_adrChanged_addr), HEX);

MonitorPort.print(F("rxDelay: 0x"));
MonitorPort.println(readMemoryUint8(local_rxDelay_addr), HEX);

MonitorPort.print(F("margin: 0x"));
MonitorPort.println(readMemoryUint8(local_margin_addr), HEX);

MonitorPort.print(F("ladrAns: 0x"));
MonitorPort.println(readMemoryUint8(local_ladrAns_addr), HEX);

MonitorPort.print(F("devsAns: 0x"));
MonitorPort.println(readMemoryUint8(local_devsAns_addr), HEX);

MonitorPort.print(F("adrEnabled: 0x"));
MonitorPort.println(readMemoryUint8(local_adrEnabled_addr), HEX);

MonitorPort.print(F("moreData: 0x"));
MonitorPort.println(readMemoryUint8(local_moreData_addr), HEX);

MonitorPort.print(F("dutyCapAns: 0x"));
MonitorPort.println(readMemoryUint8(local_dutyCapAns_addr), HEX);

MonitorPort.print(F("snchAns: 0x"));
MonitorPort.println(readMemoryUint8(local_snchAns_addr), HEX);

MonitorPort.print(F("dn2Dr: 0x"));
MonitorPort.println(readMemoryUint8(local_dn2Dr_addr), HEX);

MonitorPort.print(F("dn2Ans: 0x"));
MonitorPort.println(readMemoryUint8(local_dn2Ans_addr), HEX);

MonitorPort.print(F("missedBcns: 0x"));
MonitorPort.println(readMemoryUint8(local_missedBcns_addr), HEX);

MonitorPort.print(F("bcninfoTries: 0x"));
MonitorPort.println(readMemoryUint8(local_bcninfoTries_addr), HEX);

MonitorPort.print(F("pingSetAns: 0x"));
MonitorPort.println(readMemoryUint8(local_pingSetAns_addr), HEX);

MonitorPort.print(F("txCnt: 0x"));
MonitorPort.println(readMemoryUint8(local_txCnt_addr), HEX);

MonitorPort.print(F("txrxFlags: 0x"));
MonitorPort.println(readMemoryUint8(local_txrxFlags_addr), HEX);

MonitorPort.print(F("dataBeg: 0x"));
MonitorPort.println(readMemoryUint8(local_dataBeg_addr), HEX);

MonitorPort.print(F("dataLen: 0x"));
MonitorPort.println(readMemoryUint8(local_dataLen_addr), HEX);

MonitorPort.print(F("bcnChnl: 0x"));
MonitorPort.println(readMemoryUint8(local_bcnChnl_addr), HEX);

MonitorPort.print(F("bcnRxsyms: 0x"));
MonitorPort.println(readMemoryUint8(local_bcnRxsyms_addr), HEX);

MonitorPort.print(F("drift: 0x"));
MonitorPort.println(readMemoryInt16(local_drift_addr), HEX);

MonitorPort.print(F("lastDriftDiff: 0x"));
MonitorPort.println(readMemoryInt16(local_lastDriftDiff_addr), HEX);

MonitorPort.print(F("maxDriftDiff: 0x"));
MonitorPort.println(readMemoryInt16(local_maxDriftDiff_addr), HEX);

MonitorPort.print(F("rps: 0x"));
MonitorPort.println(readMemoryUint16(local_rps_addr), HEX);

MonitorPort.print(F("channelMap: 0x"));
MonitorPort.println(readMemoryUint16(local_channelMap_addr), HEX);

MonitorPort.print(F("opmode: 0x"));
MonitorPort.println(readMemoryUint16(local_opmode_addr), HEX);

MonitorPort.print(F("clockError: 0x"));
MonitorPort.println(readMemoryUint16(local_clockError_addr), HEX);

MonitorPort.print(F("devNonce: 0x"));
MonitorPort.println(readMemoryUint16(local_devNonce_addr), HEX);

MonitorPort.print(F("txend: 0x"));
MonitorPort.println(readMemoryInt32(local_txend_addr), HEX);

MonitorPort.print(F("rxtime: 0x"));
MonitorPort.println(readMemoryInt32(local_rxtime_addr), HEX);

MonitorPort.print(F("globalDutyAvail: "));
MonitorPort.println(readMemoryInt32(local_globalDutyAvail_addr), DEC);

MonitorPort.print(F("bcnRxtime: 0x"));
MonitorPort.println(readMemoryInt32(local_bcnRxtime_addr), HEX);

MonitorPort.print(F("freq: "));
MonitorPort.println(readMemoryUint32(local_freq_addr), DEC);

MonitorPort.print(F("netid: 0x"));
MonitorPort.println(readMemoryUint32(local_netid_addr), HEX);

MonitorPort.print(F("devaddr: 0x"));
MonitorPort.println(readMemoryUint32(local_devaddr_addr), HEX);

MonitorPort.print(F("seqnoDn: "));
MonitorPort.println(readMemoryUint32(local_seqnoDn_addr), DEC);

MonitorPort.print(F("seqnoUp: "));
MonitorPort.println(readMemoryUint32(local_seqnoUp_addr), DEC);

MonitorPort.print(F("dn2Freq: "));
MonitorPort.println(readMemoryUint32(local_dn2Freq_addr), DEC);

MonitorPort.print(F("sentpayloads: "));
MonitorPort.println(readMemoryUint32(local_sentpayloads_addr), DEC);

MonitorPort.print(F("FUPpayloads: "));
MonitorPort.println(readMemoryUint32(local_FUPpayloads_addr), DEC);
}


void print_channels_FRAM()
{
  uint8_t index;

  for (index = 0; index <= (MAX_CHANNELS - 1); index++)
  {
    MonitorPort.print(F("Channel_"));
    MonitorPort.print(index);
    MonitorPort.print(F(": "));
    MonitorPort.println( readMemoryUint32( local_channelFreq_base + (index * 4)));
  }
}

void restore_fixed_session_data()
{
  uint8_t index;
  uint8_t saved_artKey[16];
  uint8_t saved_nwkKey[16];
  uint32_t saved_devaddr;
  uint32_t saved_netid;

  for (index = 0; index <= sizeof(LMIC.artKey) - 1; index++)
  {
    saved_artKey[index] = readMemoryUint8(local_artKey_addr + index);
  }

  for (index = 0; index <= sizeof(LMIC.nwkKey) - 1; index++)
  {
    saved_nwkKey[index] = readMemoryUint8(local_nwkKey_addr + index);
  }
 
  saved_netid = readMemoryUint32(local_netid_addr);
  saved_devaddr = readMemoryUint32(local_devaddr_addr);

  LMIC_setSession (saved_netid, saved_devaddr, saved_nwkKey, saved_artKey);
  LMIC.devNonce = readMemoryUint16(local_devNonce_addr);
}


void restore_variable_session_data()
{
LMIC.rssi = readMemoryInt8(local_rssi_addr);
LMIC.snr = readMemoryInt8(local_snr_addr);
LMIC.txpow = readMemoryInt8(local_txpow_addr);
LMIC.adrTxPow = readMemoryInt8(local_adrTxPow_addr);
LMIC.adrAckReq = readMemoryInt8(local_adrAckReq_addr);

LMIC.rxsyms = readMemoryUint8(local_rxsyms_addr);
LMIC.dndr = readMemoryUint8(local_dndr_addr);
LMIC.txChnl = readMemoryUint8(local_txChnl_addr);
LMIC.globalDutyRate = readMemoryUint8(local_globalDutyRate_addr);
LMIC.upRepeat = readMemoryUint8(local_upRepeat_addr);
LMIC.datarate = readMemoryUint8(local_datarate_addr);
LMIC.errcr = readMemoryUint8(local_errcr_addr);
LMIC.rejoinCnt = readMemoryUint8(local_rejoinCnt_addr);
LMIC.pendTxPort = readMemoryUint8(local_pendTxPort_addr);
LMIC.pendTxConf = readMemoryUint8(local_pendTxConf_addr);
LMIC.pendTxLen = readMemoryUint8(local_pendTxLen_addr);
LMIC.dnConf = readMemoryUint8(local_dnConf_addr);
LMIC.adrChanged = readMemoryUint8(local_adrChanged_addr);
LMIC.rxDelay = readMemoryUint8(local_rxDelay_addr);
LMIC.margin = readMemoryUint8(local_margin_addr);
LMIC.ladrAns = readMemoryUint8(local_ladrAns_addr);
LMIC.devsAns = readMemoryUint8(local_devsAns_addr);
LMIC.adrEnabled = readMemoryUint8(local_adrEnabled_addr);
LMIC.moreData = readMemoryUint8(local_moreData_addr);
LMIC.dutyCapAns = readMemoryUint8(local_dutyCapAns_addr);
LMIC.snchAns = readMemoryUint8(local_snchAns_addr);
LMIC.dn2Dr = readMemoryUint8(local_dn2Dr_addr);
LMIC.dn2Ans = readMemoryUint8(local_dn2Ans_addr);
LMIC.missedBcns = readMemoryUint8(local_missedBcns_addr);
LMIC.bcninfoTries = readMemoryUint8(local_bcninfoTries_addr);
LMIC.pingSetAns = readMemoryUint8(local_pingSetAns_addr);
LMIC.txCnt = readMemoryUint8(local_txCnt_addr);
LMIC.txrxFlags = readMemoryUint8(local_txrxFlags_addr);
LMIC.dataBeg = readMemoryUint8(local_dataBeg_addr);
LMIC.dataLen = readMemoryUint8(local_dataLen_addr);
LMIC.bcnChnl = readMemoryUint8(local_bcnChnl_addr);
LMIC.bcnRxsyms = readMemoryUint8(local_bcnRxsyms_addr);

LMIC.drift = readMemoryInt16(local_drift_addr);
LMIC.lastDriftDiff = readMemoryInt16(local_lastDriftDiff_addr);
LMIC.maxDriftDiff = readMemoryInt16(local_maxDriftDiff_addr);

LMIC.rps = readMemoryUint16(local_rps_addr);
LMIC.channelMap = readMemoryUint16(local_channelMap_addr);
LMIC.opmode = readMemoryUint16(local_opmode_addr);
LMIC.clockError = readMemoryUint16(local_clockError_addr);
LMIC.devNonce = readMemoryUint16(local_devNonce_addr);

LMIC.txend = readMemoryInt32(local_txend_addr);
LMIC.rxtime = readMemoryInt32(local_rxtime_addr);
LMIC.globalDutyAvail = readMemoryInt32(local_globalDutyAvail_addr);
LMIC.bcnRxtime = readMemoryInt32(local_bcnRxtime_addr);

LMIC.freq = readMemoryUint32(local_freq_addr);
LMIC.netid = readMemoryUint32(local_netid_addr);
LMIC.devaddr = readMemoryUint32(local_devaddr_addr);
LMIC.seqnoDn = readMemoryUint32(local_seqnoDn_addr);
LMIC.seqnoUp = readMemoryUint32(local_seqnoUp_addr);
LMIC.dn2Freq = readMemoryUint32(local_dn2Freq_addr);
sentpayloads = readMemoryUint32(local_sentpayloads_addr);
FUPpayloads = readMemoryUint32(local_FUPpayloads_addr);

}


void restore_channels()
{
  uint8_t index;

  for (index = 0; index <= (MAX_CHANNELS - 1); index++)
  {
    LMIC.channelFreq[index] = readMemoryUint32(local_channelFreq_base + (index * 4));
  }
}
