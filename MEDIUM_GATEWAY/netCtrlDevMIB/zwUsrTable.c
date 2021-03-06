/*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "zwUsrTable.h"
#include "zwParaAccess.h"
#include "osa_debug.h"
#include "public.h"

/** Initializes the zwUsrTable module */
void
init_zwUsrTable(void)
{
    OSA_DBG_MSGX(" ");
    initUsrNumTab();

  /* here we initialize all the tables we're planning on supporting */
    initialize_table_zwUsrTable();
}

/** Initialize the zwUsrTable table by defining its contents and how it's structured */
void
initialize_table_zwUsrTable(void)
{
    const oid zwUsrTable_oid[] = {1,3,6,1,4,1,589,1,2,2};
    const size_t zwUsrTable_oid_len   = OID_LENGTH(zwUsrTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_iterator_info           *iinfo;
    netsnmp_table_registration_info *table_info;

    DEBUGMSGTL(("zwUsrTable:init", "initializing table zwUsrTable\n"));

    reg = netsnmp_create_handler_registration(
              "zwUsrTable",     zwUsrTable_handler,
              zwUsrTable_oid, zwUsrTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_OCTET_STR,  /* index: usrNumber */
                           0);
    table_info->min_column = COLUMN_CHANID;
    table_info->max_column = COLUMN_SECNUM;
    
    iinfo = SNMP_MALLOC_TYPEDEF( netsnmp_iterator_info );
    iinfo->get_first_data_point = zwUsrTable_get_first_data_point;
    iinfo->get_next_data_point  = zwUsrTable_get_next_data_point;
    iinfo->table_reginfo        = table_info;
    
    netsnmp_register_table_iterator( reg, iinfo );

    /* Initialise the contents of the table here */
}

    /* Typical data structure for a row entry */
struct zwUsrTable_entry {
    /* Index values */
    char usrNumber[USR_NUM_LEN];
    size_t usrNumber_len;

    /* Column values */
    long chanId;
    long old_chanId;
    //char usrNumber[USR_NUM_LEN];
    //size_t usrNumber_len;
    char old_usrNumber[USR_NUM_LEN];
    size_t old_usrNumber_len;
    char secNum[SEC_NUM_LEN];
    size_t secNum_len;
    char old_secNum[SEC_NUM_LEN];
    size_t old_secNum_len;

    /* Illustrate using a simple linked list */
    int   valid;
    struct zwUsrTable_entry *next;
};

struct zwUsrTable_entry  *zwUsrTable_head = NULL;

/* create a new row in the (unsorted) table */
struct zwUsrTable_entry *
zwUsrTable_createEntry(
                 char* usrNumber,
                 size_t usrNumber_len
                ) {
    struct zwUsrTable_entry *entry;

    entry = SNMP_MALLOC_TYPEDEF(struct zwUsrTable_entry);
    if (!entry)
        return NULL;

    memset(entry,0x00,sizeof(struct zwUsrTable_entry));
    memcpy(entry->usrNumber, usrNumber, usrNumber_len);
    entry->usrNumber_len = usrNumber_len;
    entry->next = zwUsrTable_head;
    zwUsrTable_head = entry;
    return entry;
}

/* remove a row from the table */
void
zwUsrTable_removeEntry( struct zwUsrTable_entry *entry ) {
    struct zwUsrTable_entry *ptr, *prev;

    if (!entry)
        return;    /* Nothing to remove */

    for ( ptr  = zwUsrTable_head, prev = NULL;
          ptr != NULL;
          prev = ptr, ptr = ptr->next ) {
        if ( ptr == entry )
            break;
    }
    if ( !ptr )
        return;    /* Can't find it */

    if ( prev == NULL )
        zwUsrTable_head = ptr->next;
    else
        prev->next = ptr->next;

    SNMP_FREE( entry );   /* XXX - release any other internal resources */
}


static void fillZwUsrNumEntry(struct zwUsrTable_entry *entry, zwUsrNum_type *item){
	if(!entry || !item){
		OSA_ERROR("Invalid Para");
		return;
	}

	bcd_to_string(item->usrNum,entry->usrNumber,BCD_USR_NUM_LEN);
	entry->usrNumber_len= BCD_USR_NUM_LEN;//strlen(entry->usrNumber);
	bcd_to_string(item->secNum,entry->secNum,BCD_SEC_NUM_LEN);
	entry->secNum_len = BCD_SEC_NUM_LEN;//strlen(entry->secNum);
	entry->chanId = item->chanId;
	
	OSA_DBG_MSGX("usrNum=%s, secNum=%s, chanId=%d",entry->usrNumber, entry->secNum, entry->chanId);
}

static void loadZwUsrNumTab(){
	int itemCnt = -1, i;
	zwUsrNum_type *tab = NULL;
	struct zwUsrTable_entry *entry = NULL;
	char usrNumber[USR_NUM_LEN];

	tab = getUsrNumTab(&itemCnt);
	OSA_DBG_MSGX(" itemCnt=%d", itemCnt);
	if(!tab || itemCnt <=0){
		OSA_ERROR("Can't load ZW Usr Num Table");
		return;
	}

	if(zwUsrTable_head == NULL){
		for(i = 0; i < itemCnt; i++){
			bcd_to_string(tab[i].usrNum,usrNumber,BCD_USR_NUM_LEN);
			zwUsrTable_createEntry(usrNumber, BCD_USR_NUM_LEN);
		}
	}	
	entry = zwUsrTable_head;
	i = 0;
	while(entry){
		fillZwUsrNumEntry(entry,&tab[i]);
		i++;
		entry = entry->next;
	}
}

static int needClear = 1;
static void clearTab() {
	struct zwUsrTable_entry *entry = NULL;

	while (zwUsrTable_head) {
		entry = zwUsrTable_head->next;
		free(zwUsrTable_head);
		zwUsrTable_head = entry;
	}
}

/* Example iterator hook routines - using 'get_next' to do most of the work */
netsnmp_variable_list *
zwUsrTable_get_first_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
	OSA_DBG_MSGX(" ");

	if (needClear) //clear table at first
		clearTab();
	needClear = 0;
	
	loadZwUsrNumTab();

    *my_loop_context = zwUsrTable_head;
    return zwUsrTable_get_next_data_point(my_loop_context, my_data_context,
                                    put_index_data,  mydata );
}

netsnmp_variable_list *
zwUsrTable_get_next_data_point(void **my_loop_context,
                          void **my_data_context,
                          netsnmp_variable_list *put_index_data,
                          netsnmp_iterator_info *mydata)
{
    struct zwUsrTable_entry *entry = (struct zwUsrTable_entry *)*my_loop_context;
    netsnmp_variable_list *idx = put_index_data;

    if ( entry ) {
        snmp_set_var_value( idx, entry->usrNumber, sizeof(entry->usrNumber) );
        idx = idx->next_variable;
        *my_data_context = (void *)entry;
        *my_loop_context = (void *)entry->next;
        return put_index_data;
    } else {
        return NULL;
    }
}


/** handles requests for the zwUsrTable table */
int
zwUsrTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    struct zwUsrTable_entry          *table_entry;

	int ret;

    DEBUGMSGTL(("zwUsrTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            table_entry = (struct zwUsrTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);

			if ( !table_entry ) {
				needClear = 1; // this loop is end, need to clear table next time
				OSA_DBG_MSGX(" ");
			}

            switch (table_info->colnum) {
            case COLUMN_CHANID:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->chanId);
                break;
            case COLUMN_USRNUMBER:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->usrNumber,
                                          table_entry->usrNumber_len);
                break;
            case COLUMN_SECNUM:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->secNum,
                                          table_entry->secNum_len);
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            table_entry = (struct zwUsrTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CHANID:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_USRNUMBER:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->usrNumber));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_SECNUM:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->secNum));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            default:
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
        for (request=requests; request; request=request->next) {
            table_entry = (struct zwUsrTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CHANID:
                table_entry->old_chanId = table_entry->chanId;
                table_entry->chanId     = *request->requestvb->val.integer;
                break;
            case COLUMN_USRNUMBER:
                memcpy( table_entry->old_usrNumber,
                        table_entry->usrNumber,
                        sizeof(table_entry->usrNumber));
                table_entry->old_usrNumber_len =
                        table_entry->usrNumber_len;
                memset( table_entry->usrNumber, 0,
                        sizeof(table_entry->usrNumber));
                memcpy( table_entry->usrNumber,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->usrNumber_len =
                        request->requestvb->val_len;
                break;
            case COLUMN_SECNUM:
                memcpy( table_entry->old_secNum,
                        table_entry->secNum,
                        sizeof(table_entry->secNum));
                table_entry->old_secNum_len =
                        table_entry->secNum_len;
                memset( table_entry->secNum, 0,
                        sizeof(table_entry->secNum));
                memcpy( table_entry->secNum,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->secNum_len =
                        request->requestvb->val_len;
                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            table_entry = (struct zwUsrTable_entry *)
                              netsnmp_extract_iterator_context(request);
            table_info  =     netsnmp_extract_table_info(      request);
    
            switch (table_info->colnum) {
            case COLUMN_CHANID:
                table_entry->chanId     = table_entry->old_chanId;
                table_entry->old_chanId = 0;
                break;
            case COLUMN_USRNUMBER:
                memcpy( table_entry->usrNumber,
                        table_entry->old_usrNumber,
                        sizeof(table_entry->usrNumber));
                memset( table_entry->old_usrNumber, 0,
                        sizeof(table_entry->usrNumber));
                table_entry->usrNumber_len =
                        table_entry->old_usrNumber_len;
                break;
            case COLUMN_SECNUM:
                memcpy( table_entry->secNum,
                        table_entry->old_secNum,
                        sizeof(table_entry->secNum));
                memset( table_entry->old_secNum, 0,
                        sizeof(table_entry->secNum));
                table_entry->secNum_len =
                        table_entry->old_secNum_len;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
