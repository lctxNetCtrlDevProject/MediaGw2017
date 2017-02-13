#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>


#ifndef DEBUG
#define DEBUG(...) \
  do \
  { \
  snmp_log(LOG_WARNING, __VA_ARGS__); \
  snmp_log(LOG_WARNING, "\n"); \
  } \
  while(0);
#endif

#ifndef ERROR
#define ERROR(...) \
  do \
  { \
  snmp_log(LOG_WARNING, "\n ERROR  (%s | %s | %d): \n\t", __FILE__, __func__, __LINE__); \
  snmp_log(LOG_WARNING, __VA_ARGS__); \
  snmp_log(LOG_WARNING, "\n"); \
  } \
  while(0);
#endif


#endif
