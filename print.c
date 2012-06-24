/*
* $Id: print.c,v 1.3 2005/10/28 02:56:22 jms Exp $
*
* Revision History
* ===================
* $Log: print.c,v $
* Revision 1.3  2005/10/28 02:56:22  jms
* add platform-specific printf formats to allow for DSS_HUGE data type
*
* Revision 1.2  2005/01/03 20:08:59  jms
* change line terminations
*
* Revision 1.1.1.1  2004/11/24 23:31:47  jms
* re-establish external server
*
* Revision 1.4  2004/02/18 16:26:49  jms
* 32/64 bit changes for overflow handling needed additional changes when ported back to windows
*
* Revision 1.3  2004/02/18 14:05:53  jms
* porting changes for LINUX and 64 bit RNG
*
* Revision 1.2  2004/01/22 05:49:29  jms
* AIX porting (AIX 5.1)
*
* Revision 1.1.1.1  2003/08/07 17:58:34  jms
* recreation after CVS crash
*
* Revision 1.2  2003/08/07 17:58:34  jms
* Convery RNG to 64bit space as preparation for new large scale RNG
*
* Revision 1.1.1.1  2003/04/03 18:54:21  jms
* initial checkin
*
*
*/
/* generate flat files for data load */
#include <stdio.h>
#ifndef VMS
#include <sys/types.h>
#endif
#if defined(SUN)
#include <unistd.h>
#endif
#include <math.h>

#include "dss.h"
#include "dsstypes.h"
#include <string.h>

/*
 * Function Prototypes
 */
FILE *print_prep PROTO((int table, int update));
int pr_drange PROTO((int tbl, DSS_HUGE min, DSS_HUGE cnt, long num));

FILE *
print_prep(int table, int update)
{
	char upath[128];
	FILE *res;

	if (updates)
		{
		if (update > 0) /* updates */
			if ( insert_segments )
				{
				int this_segment;
				if(strcmp(tdefs[table].name,"orders.tbl"))
					this_segment=++insert_orders_segment;
				else 
					this_segment=++insert_lineitem_segment;
				sprintf(upath, "%s%c%s.u%d.%d", 
					env_config(PATH_TAG, PATH_DFLT),
					PATH_SEP, tdefs[table].name, update%10000,this_segment);
				}
			else
				{
				sprintf(upath, "%s%c%s.u%d",
				env_config(PATH_TAG, PATH_DFLT),
				PATH_SEP, tdefs[table].name, update);
				}
		else /* deletes */
			if ( delete_segments )
				{
				++delete_segment;
				sprintf(upath, "%s%cdelete.u%d.%d",
					env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update%10000,
					delete_segment);
				}
			else
				{
				sprintf(upath, "%s%cdelete.%d",
				env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update);
				}
		return(fopen(upath, "w"));
        }
    res = tbl_open(table, "w");
    OPEN_CHECK(res, tdefs[table].name);
    return(res);
}

int
dbg_print_separator(char* target)
{
	char* temp = malloc(sizeof(char));
	sprintf(temp, "%c", SEPARATOR);
	strcat(target, temp);
	free(temp);
}

int dbg_print(int format, char *target, void *data, int len, int sep) {
	int dollars, cents;
	char * temp = malloc(1024);

	switch (format) {
	case DT_STR:
		sprintf(temp, "%s", (char *) data);
		break;
#ifdef MVS
	case DT_VSTR:
		/* note: only used in MVS, assumes columnar output */
		sprintf(temp, "%c%c%-*s", (len >> 8) & 0xFF, len & 0xFF, len,
				(char *) data);
		break;
#endif /* MVS */
	case DT_INT:
		sprintf(temp, "%ld", (long) data);
		break;
	case DT_HUGE:
		sprintf(temp, HUGE_FORMAT, *(DSS_HUGE *) data);
		break;
	case DT_KEY:
		sprintf(temp, "%ld", (long) data);
		break;
	case DT_MONEY:
		cents = (int) *(DSS_HUGE *) data;
		if (cents < 0) {
			sprintf(temp, "-");
			cents = -cents;
		}
		dollars = cents / 100;
		cents %= 100;
		sprintf(temp, "%ld.%02ld", dollars, cents);
		break;
	case DT_CHR:
		sprintf(temp, "%c", *(char *) data);
		break;
	}

	strcat(target, temp);
	free(temp);

#ifdef EOL_HANDLING
	if (sep)
#endif /* EOL_HANDLING */

	return (0);
}

int
pr_cust(customer_t *c, int mode)
{
/*static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(CUST, 0);*/

   //PR_STRT(fp);
   PR_HUGE(&c->custkey);
   dbg_print_separator(record);
   if (scale <= 3000)
   PR_VSTR(c->name, C_NAME_LEN);
   else
   PR_VSTR(c->name, C_NAME_LEN + 3);
   dbg_print_separator(record);
   PR_VSTR(c->address, c->alen);
   dbg_print_separator(record);
   PR_HUGE(&c->nation_code);
   dbg_print_separator(record);
   PR_STR(c->phone, PHONE_LEN);
   dbg_print_separator(record);
   PR_MONEY(&c->acctbal);
   dbg_print_separator(record);
   PR_STR(c->mktsegment, C_MSEG_LEN);
   dbg_print_separator(record);
   PR_VSTR_LAST(c->comment, c->clen);
   //PR_END(fp);

   return(0);
}

/*
 * print the numbered order 
 */
int
pr_order(order_t *o, int mode)
{
    /*static FILE *fp_o = NULL;
    static int last_mode = 0;
        
    if (fp_o == NULL || mode != last_mode)
        {
        if (fp_o) 
            fclose(fp_o);
        fp_o = print_prep(ORDER, mode);
        last_mode = mode;
        }*/
    
    //PR_STRT(fp_o);
    PR_HUGE(&o->okey);
    dbg_print_separator(record);
    PR_HUGE(&o->custkey);
    dbg_print_separator(record);
    PR_CHR(&o->orderstatus);
    dbg_print_separator(record);
    PR_MONEY(&o->totalprice);
    dbg_print_separator(record);
    PR_STR(o->odate, DATE_LEN);
    dbg_print_separator(record);
    PR_STR(o->opriority, O_OPRIO_LEN);
    dbg_print_separator(record);
    PR_STR(o->clerk, O_CLRK_LEN);
    dbg_print_separator(record);
    PR_INT(o->spriority);
    dbg_print_separator(record);
    PR_VSTR_LAST(o->comment, o->clen);
    //PR_END(fp_o);

    return(0);
}

/*
 * print an order's lineitems
 */
int
pr_line(order_t *o, int mode)
{
   /* static FILE *fp_l = NULL;
    static int last_mode = 0;*/
    long      i;
        
    /*if (fp_l == NULL || mode != last_mode)
        {
        if (fp_l) 
            fclose(fp_l);
        fp_l = print_prep(LINE, mode);
        last_mode = mode;
        }*/

    for (i = 0; i < o->lines; i++)
        {
        //PR_STRT(fp_l);
        PR_HUGE(&o->l[i].okey);
        dbg_print_separator(record);
        PR_HUGE(&o->l[i].partkey);
        dbg_print_separator(record);
        PR_HUGE(&o->l[i].suppkey);
        dbg_print_separator(record);
        PR_HUGE(&o->l[i].lcnt);
        dbg_print_separator(record);
        PR_HUGE(&o->l[i].quantity);
        dbg_print_separator(record);
        PR_MONEY(&o->l[i].eprice);
        dbg_print_separator(record);
        PR_MONEY(&o->l[i].discount);
        dbg_print_separator(record);
        PR_MONEY(&o->l[i].tax);
        dbg_print_separator(record);
        PR_CHR(&o->l[i].rflag[0]);
        dbg_print_separator(record);
        PR_CHR(&o->l[i].lstatus[0]);
        dbg_print_separator(record);
        PR_STR(o->l[i].sdate, DATE_LEN);
        dbg_print_separator(record);
        PR_STR(o->l[i].cdate, DATE_LEN);
        dbg_print_separator(record);
        PR_STR(o->l[i].rdate, DATE_LEN);
        dbg_print_separator(record);
        PR_STR(o->l[i].shipinstruct, L_INST_LEN);
        dbg_print_separator(record);
        PR_STR(o->l[i].shipmode, L_SMODE_LEN);
        dbg_print_separator(record);
        PR_VSTR_LAST(o->l[i].comment,o->l[i].clen);
        //PR_END(fp_l);
        }

   return(0);
}

/*
 * print the numbered order *and* its associated lineitems
 */
int
pr_order_line(order_t *o, int mode)
{
    tdefs[ORDER].name = tdefs[ORDER_LINE].name;
    pr_order(o, mode);
    pr_line(o, mode);

    return(0);
}

/*
 * print the given part
 */
int
pr_part(part_t *part, int mode)
{
/*static FILE *p_fp = NULL;

    if (p_fp == NULL)
        p_fp = print_prep(PART, 0);*/

   //PR_STRT(p_fp);
   PR_HUGE(&part->partkey);
   dbg_print_separator(record);
   PR_VSTR(part->name,part->nlen);
   dbg_print_separator(record);
   PR_STR(part->mfgr, P_MFG_LEN);
   dbg_print_separator(record);
   PR_STR(part->brand, P_BRND_LEN);
   dbg_print_separator(record);
   PR_VSTR(part->type,part->tlen);
   dbg_print_separator(record);
   PR_HUGE(&part->size);
   dbg_print_separator(record);
   PR_STR(part->container, P_CNTR_LEN);
   dbg_print_separator(record);
   PR_MONEY(&part->retailprice);
   dbg_print_separator(record);
   PR_VSTR_LAST(part->comment,part->clen);
   //PR_END(p_fp);

   return(0);
}

/*
 * print the given part's suppliers
 */
int
pr_psupp(part_t *part, int mode)
{
   /* static FILE *ps_fp = NULL;*/
    long      i;

    /*if (ps_fp == NULL)
        ps_fp = print_prep(PSUPP, mode);*/

   for (i = 0; i < SUPP_PER_PART; i++)
      {
      //PR_STRT(ps_fp);
      PR_HUGE(&part->s[i].partkey);
      dbg_print_separator(record);
      PR_HUGE(&part->s[i].suppkey);
      dbg_print_separator(record);
      PR_HUGE(&part->s[i].qty);
      dbg_print_separator(record);
      PR_MONEY(&part->s[i].scost);
      dbg_print_separator(record);
      PR_VSTR_LAST(part->s[i].comment,part->s[i].clen);
      //PR_END(ps_fp);
      }

   return(0);
}

/*
 * print the given part *and* its suppliers
 */
int
pr_part_psupp(part_t *part, int mode)
{
    tdefs[PART].name = tdefs[PART_PSUPP].name;
    pr_part(part, mode);
    pr_psupp(part, mode);

    return(0);
}

int
pr_supp(supplier_t *supp, int mode)
{
/*static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(SUPP, mode);*/

   //PR_STRT(fp);
   PR_HUGE(&supp->suppkey);
   dbg_print_separator(record);
   PR_STR(supp->name, S_NAME_LEN);
   dbg_print_separator(record);
   PR_VSTR(supp->address, supp->alen);
   dbg_print_separator(record);
   PR_HUGE(&supp->nation_code);
   dbg_print_separator(record);
   PR_STR(supp->phone, PHONE_LEN);
   dbg_print_separator(record);
   PR_MONEY(&supp->acctbal);
   dbg_print_separator(record);
   PR_VSTR_LAST(supp->comment, supp->clen);
   //PR_END(fp);

   return(0);
}

int
pr_nation(code_t *c, int mode)
{
/*static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(NATION, mode);*/

   //PR_STRT(fp);
   PR_HUGE(&c->code);
   dbg_print_separator(record);
   PR_STR(c->text, NATION_LEN);
   dbg_print_separator(record);
   PR_INT(c->join);
   dbg_print_separator(record);
   PR_VSTR_LAST(c->comment, c->clen);
   //PR_END(fp);

   return(0);
}

int
pr_region(code_t *c, int mode)
{
/*static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(REGION, mode);*/

   //PR_STRT(fp);
   PR_HUGE(&c->code);
   dbg_print_separator(record);
   PR_STR(c->text, REGION_LEN);
   dbg_print_separator(record);
   PR_VSTR_LAST(c->comment, c->clen);
   //PR_END(fp);

   return(0);
}

/* 
 * NOTE: this routine does NOT use the BCD2_* routines. As a result,
 * it WILL fail if the keys being deleted exceed 32 bits. Since this
 * would require ~660 update iterations, this seems an acceptable
 * oversight
 */
int
pr_drange(int tbl, DSS_HUGE min, DSS_HUGE cnt, long num)
{
    static int  last_num = 0;
    static FILE *dfp = NULL;
    DSS_HUGE child = -1;
    DSS_HUGE start, last, new;

	static DSS_HUGE rows_per_segment=0;
	static DSS_HUGE rows_this_segment=0;

    if (last_num != num)
        {
        if (dfp)
            fclose(dfp);
        dfp = print_prep(tbl, -num);
        if (dfp == NULL)
            return(-1);
        last_num = num;
		rows_this_segment=0;
        }

    start = MK_SPARSE(min, num/ (10000 / UPD_PCT));
    last = start - 1;
    for (child=min; cnt > 0; child++, cnt--)
	{
		new = MK_SPARSE(child, num/ (10000 / UPD_PCT));
		if (delete_segments)
		{

			if(rows_per_segment==0) 
				rows_per_segment = (cnt / delete_segments) + 1;
			if((++rows_this_segment) > rows_per_segment)
			{
				fclose(dfp);
				dfp = print_prep(tbl, -num);
				if (dfp == NULL) return(-1);
				last_num = num;
				rows_this_segment=1;
			}
		}
		
		//PR_STRT(dfp);
		dbg_print_separator(record);
		PR_HUGE(&new);
		//PR_END(dfp);
		start = new;
		last = new;
	}
    
    return(0);
}



