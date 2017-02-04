/*
====================================================
** Description: Config file header,it's sample.
** it read config from file only
** 
====================================================
*/
#ifndef	_MGW_CONFIG_H_
#define	_MGW_CONFIG_H_

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"
{
#endif

/*
!brief
* 配置文件分为两部分，分类值和变量值
*/
struct variable
{
	struct variable	*next;		/*next variable*/
	struct variable *prev;		/*prev variable*/
	struct variable	*root;		/*first variable*/
	char			name[80];	/*variable name*/
	char			value[120];	/*str val*/
};

#define CAT_NAME  80

struct category
{
	struct category	*next;
	struct category	*root;
	struct variable	*variable_list;
	int		varcnt;
	char		name[CAT_NAME];
};

#define CFG_NAME_MAX 256 
struct config
{
	struct category	*category_list;
	int		catcnt;
	char		name[CFG_NAME_MAX];			/*config file name*/
	FILE		*fstream;				/*file  stream*/
	char*	description;			/*file  description*/
};

extern struct category* find_config_category(struct config * cfg,const char* category_name);
extern int32 rewrite_config(struct config * cfg);
extern const char* append_config_var(struct config* cfg,const char * catname,const char* varname,const char* new_val);
extern const char* add_config_var(struct config* cfg, const char * catname, const char* varname, const char* new_val);
extern int32 sub_config_var(struct config* cfg,const char * catname,const char * varname,const char* info);
extern int32 change_config_cat_var_val(struct config * cfg, const char * catname, const char* varname, int32 val);
extern char* change_config_cat_var_str(struct config * cfg, const char * catname, const char* varname, const char* new_val);
extern char* change_config_cat_var(struct config * cfg, const char * catname, const char* varname, const char* new_val);
extern int32 get_config_cat_var_val(struct config * cfg, const char * catname, const char* varname);
extern const char* get_config_cat_var_str(struct config * cfg, const char * catname, const char* varname);
extern const char* change_config_var(struct config * cfg,const char* varname,const char* new_val);
extern int32 get_config_var_val(struct config * cfg,const char* varname);
extern const char* get_config_cat_var_str(struct config * cfg, const char * catname, const char* varname);
extern const char* change_config_var(struct config * cfg,const char* varname,const char* new_val);
extern int32 get_config_var_val(struct config * cfg,const char* varname);
extern const char* get_config_var_str(struct config * cfg,const char* varname);
extern int32 parse_text_line(const char *buf,struct config* cfg);
extern char* find_config_var(struct config * cfg,const char* varname);
extern int32 load_config(struct config * cfg);
extern int32 init_config(struct config ** cfg, char *filename);
extern int32 clean_config_category(struct config* cfg,const char* catname);
extern char* change_config_var_val(struct config * cfg,const char* varname, int val);

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif	/*_MGW_CONFIG_H_*/

