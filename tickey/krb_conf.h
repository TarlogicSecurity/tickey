#ifndef KRB_CONF_C
#define KRB_CONF_C

#define KRB5_CONF_FILE "/etc/krb5.conf"

typedef struct {
	char * type;
	char * subtype;
	char * name;
} ccache_info_t;


ccache_info_t * ccache_info_from_krb_conf();
ccache_info_t * default_ccache_info();
ccache_info_t * krb5_ccache_info();
#endif
