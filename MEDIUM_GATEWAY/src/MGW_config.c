/*
============================================================
** Description: 配置文件，文件名存为mgw.conf
** Config Managment
============================================================
*/

#include "PUBLIC.h"


/* local function */
static char * skip_blank(const char* str);
static char * trim_string(const char* str);

/*
!brief
* the macro below define sample list operation
* list use to link .conf file
* the format is:
[CAT0]			#comment

variable0 = value0

......
* it's like .ini file common in Windows
*/
#define init_variable_list(list,element) \
	do{ \
	list = element; \
	element->root = element; \
	}while(0)


#define	add_variable(list,element) \
	do{ \
	list->next = element; \
	element->prev = list; \
	element->root = list->root; \
	list = element; \
	}while(0)

#define sub_variable(list,element) \
	do{ \
	if(list->root == element) \
		list->root = element->next; \
	if(element->prev) \
		element->prev->next = element->next; \
	if(element->next) \
		element->next->prev = element->prev; \
	else \
		list = element->prev; \
	}while(0)


#define init_category_list(list,element) \
	do{ \
	element->root = element; \
	list = element; \
	}while(0)

#define add_category(list,element) \
	do{ \
	list->next = element; \
	element->root = list->root; \
	list = element; \
	}while(0)


/*
!brief string common function
* skip_blank()-----skip heading blanks;
* trim_string()-----rm tail blanks;
* note: 
* param str in function trim_string() must be store in ram,
* don't use like this: trim_string("abc   ");//Segmentation fault
*
*/
static char * skip_blank(const char* str)
{
	while (*str && ((unsigned char) *str) < 33)
		str++;
	return (char *)str;

}

static char * trim_string(const char* str)
{
	char *work = (char *)str;

	if (work) {
		work += strlen(work) - 1;
		/* It's tempting to only want to erase after we exit this loop, 
		   but since inc_trim_blanks *could* receive a constant string
		   (which we presumably wouldn't have to touch), we shouldn't
		   actually set anything unless we must, and it's easier just
		   to set each position to \0 than to keep track of a variable
		   for it */
		while ((work >= str) && ((unsigned char) *work) < 33)
			*(work--) = '\0';
	}
	return (char *)str;

}

int32 parse_text_line(const char *buf,struct config* cfg)
{
	char *cur = NULL;
	char *c = NULL;
	uint8 len = 0;
	char *pt = NULL;
	
	if((cur = skip_blank(buf)) != NULL)
	{
		if(*cur == '[')
		{
			struct category *newcat = NULL;
			char *catname;
			cur++;
			c = strchr(cur,']');
			if(!c)
			{
				VERBOSE_OUT(LOG_SYS,"Find '[' But not Find ']'\n");
				return DRV_ERR;
			}
			
			*c++ = '\0';
			catname = cur;
			newcat = calloc(1,sizeof(struct category));
			len = strlen(catname);
			if(len >= sizeof(newcat->name))
			{
				free(newcat);
				printf("cat name too long %d\r\n", len);
				return DRV_ERR;
			}
			
			strncpy((char *)newcat->name, catname, len);
			/*list operator*/
			if(cfg->category_list != NULL)
			{
				add_category(cfg->category_list,newcat);
			}
			else
			{
				init_category_list(cfg->category_list,newcat);
				cfg->category_list->variable_list = NULL;
			}
			
			cfg->catcnt++;
			return DRV_OK;
		}
		else
		{
			struct variable *newvar = NULL;
			char *comment = NULL;
			char *varname;
			
			comment = strchr(cur,'#');
			if(comment)
			{	
				//*comment = '\0';
				newvar = calloc(1,sizeof(struct variable));
				memset(newvar, 0x00, sizeof(struct variable));

				//trim_string(comment);
				//strcpy(newvar->name,comment);
				
				pt = trim_string(comment);
				if(strlen(pt) >= sizeof(newvar->name))
				{
					free(newvar);
					return DRV_ERR;
				}
				
				strncpy((char *)newvar->name, pt, sizeof(newvar->name));
				
				*((unsigned int *)&newvar->value[0]) = 0x5a5a5a5a;
				
				/*list operator add variable to category*/
				if(cfg->category_list->variable_list != NULL)
					add_variable(cfg->category_list->variable_list,newvar);
				else
					init_variable_list(cfg->category_list->variable_list,newvar);
					
				cfg->category_list->varcnt++;
				return DRV_OK;
			}
			
			c = strchr(cur,'=');
			if(c)
			{
				*c++ = '\0';
				varname = trim_string(cur);
				if((c = skip_blank(c)) != NULL)
				{
					comment = strchr(c,'#');
					if(comment)
						*comment = '\0';
					
					newvar = calloc(1,sizeof(struct variable));
					//strcpy(newvar->name,varname);
					//strcpy(newvar->value,trim_string(c));
					
					if(strlen(varname) > sizeof(newvar->name))
					{
						free(newvar);
						return DRV_ERR;
					}
					
					strncpy((char *)newvar->name,varname, sizeof(newvar->name));

					//strcpy(newvar->value,trim_string(c));
					pt = trim_string(c);
					if(strlen(pt) >= sizeof(newvar->value))
					{
						free(newvar);
						return DRV_ERR;
					}
					
					strncpy((char *)newvar->value, pt, sizeof(newvar->value));
					
					/*list operator add variable to category*/
					if(cfg->category_list->variable_list != NULL)
					{
						add_variable(cfg->category_list->variable_list,newvar);
					}
					else
					{
						init_variable_list(cfg->category_list->variable_list,newvar);
					}	

					cfg->category_list->varcnt++;
					
					return DRV_OK;
				}
			}
		}
	}

	return DRV_OK;
}

/*
!brief
* find variable with special name,case sensitive
* return value is always string
*/

char* find_config_var(struct config * cfg,const char* varname)
{
	/*find special variable*/
	if(!cfg || !varname)
		return NULL;
	{
		struct category * iterator_cat;
		struct variable * iterator_var;
		if(!cfg->catcnt)
			return NULL;
		for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
		{
			if(iterator_cat->varcnt == 0)
				continue;
			for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
				if(strcmp(iterator_var->name,varname) == 0)
					return iterator_var->value;
		}
	}
	return NULL;
}


/*
!brief
* get config variable value in string format
* get config variable value in integer format
*/

const char* get_config_var_str(struct config * cfg,const char* varname)
{
	return find_config_var(cfg,varname);
}

int32 get_config_var_val(struct config * cfg,const char* varname)
{
	char * tmp_res;
	tmp_res = find_config_var(cfg,varname);
	if(tmp_res != NULL)
	{
		return atoi(tmp_res);
	}
	else
	{
		return DRV_OK;
	}	
}


/*
!brief
* find variable with special name,case sensitive
* return value is always string
*/
static char* find_config_cat_var(struct config * cfg, const char * catname, const char* varname)
{
	/*find special variable*/
	if(!cfg || !varname)
	{
		return NULL;
	}
	
	struct category *iterator_cat = NULL;
	struct variable *iterator_var = NULL;

	if(!cfg->catcnt)
	{
		return NULL;
	}
	
	for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		if(iterator_cat->varcnt == 0)
		{
			continue;
		}
		
		if(0 != strcmp(iterator_cat->name, catname))
		{
			//printf("iterator_cat->name = %s\r\n", iterator_cat->name);
			continue;
		}
		
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{		
			if(strcmp(iterator_var->name,varname) == 0)
			{
				return iterator_var->value;
			}
		}
	}

	return NULL;
}
/*
!brief
* get config variable value in string format
* get config variable value in integer format
*/
const char* get_config_cat_var_str(struct config * cfg, const char * catname, const char* varname)
{
	return find_config_cat_var(cfg, catname, varname);
}

int32 get_config_cat_var_val(struct config * cfg, const char * catname, const char* varname)
{
	char * tmp_res = NULL;
	tmp_res = find_config_cat_var(cfg, catname, varname);
	if(tmp_res != NULL)
	{
		//printf("tmp_res  = %s\r\n", tmp_res);
		return atoi(tmp_res);
	}
	else
	{
		return DRV_OK;
	}

}

/*
!brief
* change variable with special name,name is case sensitive
* return value is string which pass to the function
*/
const char* change_config_var(struct config * cfg,const char* varname,const char* new_val)
{
	
	/*parameter check*/
	if(!cfg || !varname)
	{
		return NULL;
	}

	struct category  *iterator_cat = NULL;
	struct variable  *iterator_var = NULL;
	if(!cfg->catcnt)
	{
		return NULL;
	}
	
	for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		if(iterator_cat->varcnt == 0)
		{
			continue;
		}
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{
			if(strcmp(iterator_var->name,varname) == 0)
			{
				if(strlen(new_val) >= sizeof(iterator_var->value))
				{
					return NULL;
				}
				
				strncpy((char *)iterator_var->value, new_val, sizeof(iterator_var->value));

				return iterator_var->value;
			}
		}
	}

	return NULL;

}

char* change_config_var_val(struct config * cfg,const char* varname, int val)
{
	char buf[10] = {0};
	const char *p = NULL;

	snprintf(buf, sizeof(buf), "%d", val);
	
	p = change_config_var(cfg, varname, (char *)buf);
	if(NULL == p)
	{
		return NULL;
	}
	
	return NULL;

}

char* change_config_cat_var(struct config * cfg, const char * catname, const char* varname, const char* new_val)
{
	/*parameter check*/
	if(!cfg || !varname)
	{
		printf("%s: error \r\n", __func__);
		return NULL;
	}
	
	struct category * iterator_cat;
	struct variable * iterator_var;
	if(!cfg->catcnt)
	{
		printf("%s: cfg->catcnt = %d error \r\n", __func__, cfg->catcnt);
		return NULL;
	}
	
	for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		if(iterator_cat->varcnt == 0)
		{	
			continue;
		}
		
		if(0 != strcmp(iterator_cat->name, catname))
		{
			continue;
		}
		
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{
			if(strcmp(iterator_var->name,varname) == 0)
			{
				if(strlen(new_val) > sizeof(iterator_var->value))
				{
					return NULL;
				}
				
				memset(iterator_var->value, 0x00, sizeof(iterator_var->value));

				strncpy((char *)iterator_var->value,new_val, sizeof(iterator_var->value));

				return iterator_var->value;
			}
		}
	}
	
	return NULL;
}


char* change_config_cat_var_str(struct config * cfg, const char * catname, const char* varname, const char* new_val)
{
	char *p = NULL;
	p = change_config_cat_var(cfg, catname, varname, new_val);

	return p;
}

int32 change_config_cat_var_val(struct config * cfg, const char * catname, const char* varname, int32 val)
{
	char buf[10] = {0};
	char *p = NULL;

	snprintf(buf, sizeof(buf), "%d", val);
	
	p = change_config_cat_var(cfg, catname, varname, buf);
	if(NULL == p)
	{
		return DRV_ERR;
	}
	
	return DRV_OK;
}

const char* append_config_var(struct config* cfg, const char * catname, const char* varname, const char* new_val)
{
	char *pt = NULL;
	struct category *newcat = NULL;
       int len;
	/*parameter check*/
	if(!cfg || !catname || !varname || !new_val)
		return NULL;
	if (NULL==find_config_category(cfg,catname))
	{
		newcat = calloc(1,sizeof(struct category));
		len = strlen(catname);
		if(len >= 80)
		{
			free(newcat);
			printf("cat name too long %d\r\n", len);
			return NULL;
		}
		
		strncpy((char *)newcat->name, catname, sizeof(newcat->name));

		add_category(cfg->category_list,newcat);
	}		

	struct category * iterator_cat;
	struct variable * newvar;
	if(!cfg->catcnt)
		return NULL;
	for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{
		if(!strcmp(iterator_cat->name,catname))
		{
			newvar = calloc(1,sizeof(struct variable));
			snprintf(newvar->name, sizeof(newvar->name), "%s",varname);
			
			pt = trim_string(new_val);
			if(strlen(pt) >= sizeof(newvar->value))
			{
				free(newvar);
				return NULL;
			}

			strncpy((char *)newvar->value, pt, sizeof(newvar->value));
			//strcpy(newvar->value,trim_string(new_val));
			/*list operator add variable to category*/
			if(iterator_cat->variable_list != NULL)
				add_variable(iterator_cat->variable_list,newvar);

			else
				init_variable_list(iterator_cat->variable_list,newvar);
			iterator_cat->varcnt++;
			return newvar->name;
		}
	}

	return NULL;

}


int32 clean_config_category(struct config* cfg,const char* catname)
{
	/*parameter check*/
	if(!cfg || !catname)
		return (-1);
	
	{
		struct category * iterator_cat;
		struct variable * iterator_var;
		struct variable * var;
		if(!cfg->catcnt)
			return (-1);
		for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
		{
			if(!strcmp(iterator_cat->name,catname))
			{
				for(iterator_var=iterator_cat->variable_list->root;iterator_var;)
				{
					var = iterator_var;
					iterator_var=iterator_var->next;
					free(var);
				}
				iterator_cat->varcnt=0;
				iterator_cat->variable_list=NULL;
				return DRV_OK;
			}
		}
	}

	return DRV_OK;

}
const char* add_config_var(struct config* cfg,const char * catname,const char* varname,const char* new_val)
{
	char *pt = NULL;
	/*parameter check*/
	if(!cfg || !catname || !varname || !new_val)
		return NULL;
	
	{
		struct category * iterator_cat;
		struct variable * newvar;
		if(!cfg->catcnt)
			return NULL;
		for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
		{
			if(!strcmp(iterator_cat->name,catname))
			{
				newvar = calloc(1,sizeof(struct variable));
				
				snprintf(newvar->name, sizeof(newvar->name), "%s", varname);

				pt = trim_string(new_val);
				if(strlen(pt) >= sizeof(newvar->value))
				{
					free(newvar);
					return NULL;
				}

				strncpy((char *)newvar->value, pt, sizeof(newvar->value));

				//strcpy(newvar->value,trim_string(new_val));	
				/*list operator add variable to category*/
				if(iterator_cat->variable_list != NULL)
					add_variable(iterator_cat->variable_list,newvar);
				else
					init_variable_list(iterator_cat->variable_list,newvar);
				iterator_cat->varcnt++;
				return newvar->name;
			}
		}
	}
	return NULL;

}


int32 sub_config_var(struct config* cfg,const char * catname,const char * varname,const char* info)
{
	/*!brief
	* find info in variable follow with category and remove it from category_list
	*/
	/*parameter check*/
	if(!cfg || !catname || !varname || !info)
	{
		return DRV_ERR;
	}
	
	{
		struct category * iterator_cat;
		struct variable * iterator_var;
//		struct variable * var;
		if(!cfg->catcnt)
		{
			return DRV_ERR;
		}
		
		for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
		{
			if(!strcmp(iterator_cat->name,catname))
			{
				if(!iterator_cat->varcnt)
					return(-1);
				for(iterator_var=iterator_cat->variable_list->root;iterator_var;)
				{
					if(strncasecmp(iterator_var->name,varname,strlen(varname)) == 0){
						struct variable * found;
						sub_variable(iterator_cat->variable_list,iterator_var);
						found = iterator_var;
						free(found);
						iterator_cat->varcnt--;
							return 0;
					}
					iterator_var = iterator_var->next;
				}
				return DRV_OK;
			}
		}
	}

	return DRV_OK;
}

/*
!brief
* return all variable within special category
*/
struct category* find_config_category(struct config * cfg,const char* category_name)
{
	if(!cfg || !category_name)
		return NULL;
	{
		struct category * iterator_cat;
		if(!cfg->catcnt)
			return NULL;

		for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
			if(strcmp(iterator_cat->name,category_name) == 0)
				return iterator_cat;
	}
	return NULL;
}


int32 rewrite_config(struct config * cfg)
{
	FILE file ;
	FILE* fstream = NULL;
	
	int cnt = 0;

	memset(&file, 0x00, sizeof(file));
	fstream = &file;
	if(!cfg->catcnt)
	{
		return DRV_ERR;
	}
	
	printf("update config file, %s\r\n", cfg->name);
	
	fstream = fopen(cfg->name,"w");

	if(!fstream)
	{
		VERBOSE_OUT(LOG_SYS,"Can't Open %s\n",cfg->name);
		//fclose(fstream);
		return DRV_ERR;
	}

	//fprintf(fstream,"%s","[note]\n#Z024 machine config_II.conf\n#config system paramer\n");
	
	struct category * iterator_cat;
	struct variable * iterator_var;
	
	
	for(iterator_cat=cfg->category_list->root;iterator_cat;iterator_cat=iterator_cat->next)
	{	
		/*category first*/
		fprintf(fstream,"[%s]\n",iterator_cat->name);
		/*variable list*/
		cnt = 0;

		if(iterator_cat->varcnt == 0)
		{
			fprintf(fstream,"\n");
			continue;
		}
			

		//printf("iterator_cat->name = %s\r\n", iterator_cat->name);
		
		for(iterator_var=iterator_cat->variable_list->root;iterator_var;iterator_var=iterator_var->next)
		{
			//printf("iterator_var->name = %s\r\n", iterator_var->name);
		
			if(0x5a5a5a5a != *((unsigned int *)iterator_var->value))
			{
				cnt++;
				fprintf(fstream,"%s = %s\n",iterator_var->name,iterator_var->value);
			}
			else
			{	if(0 != cnt)
				{
					fprintf(fstream,"\n");
				}
				fprintf(fstream,"%s\n",iterator_var->name);
			}
		}
		
		fprintf(fstream,"\n");
	}
	
	fclose(fstream);
	
	return DRV_OK;
}


int32 load_config(struct config * cfg)
{
	FILE* fstream = NULL;
	char	line_text[256] = {0};
	
	fstream = fopen(cfg->name,"r");
	if(!fstream)
	{
		VERBOSE_OUT(LOG_SYS,"Can't Open %s\n",cfg->name);
		return DRV_ERR;
	}

	while(fgets(line_text,sizeof(line_text),fstream))
	{
		parse_text_line(line_text,cfg);
	}
	fclose(fstream);

	return DRV_OK;
}

int32 init_config(struct config **cfg, char *filename)
{
	int16 len = 0;
	int32 ret = 0;
	
	*cfg = (struct config *)calloc(1,sizeof(struct config));
	if(!*cfg)
	{
		VERBOSE_OUT(LOG_SYS,"Memory not enough\n");
		return DRV_ERR;
	}

	len = strlen(filename);
	if(len >= sizeof((*cfg)->name))
	{
		return DRV_ERR;
	}
	
	strncpy((char *)((*cfg)->name), filename, sizeof((*cfg)->name));
	(*cfg)->category_list = NULL;
	(*cfg)->catcnt = 0;	

	ret = load_config(*cfg);
	if(DRV_OK != ret)
	{
		VERBOSE_OUT(LOG_SYS,"load_config failed %d \n", ret);
		return DRV_ERR;
	}	
	
	return DRV_OK;
}


