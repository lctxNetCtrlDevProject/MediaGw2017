/*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */
#ifndef NETCTRLDEVMIB_H
#define NETCTRLDEVMIB_H

/* function declarations */
void init_netCtrlDevMIB(void);
void initialize_table_zhenRFenjTable(void);
Netsnmp_Node_Handler zhenRFenjTable_handler;
Netsnmp_First_Data_Point  zhenRFenjTable_get_first_data_point;
Netsnmp_Next_Data_Point   zhenRFenjTable_get_next_data_point;

/* column number definitions for table zhenRFenjTable */
       #define COLUMN_ZJID		2
       #define COLUMN_FENJID		3
       #define COLUMN_FENJNUM		4
#endif /* NETCTRLDEVMIB_H */
