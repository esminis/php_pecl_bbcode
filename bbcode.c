/*
 * This source file is part of the bbcode library.
 * Written and maintained by Xavier De Cock 2006-2007
 * Licensed under the BSD License Terms
 * Refer to the accompanying documentation for details on usage and license.
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <time.h>
#include "bbcode.h"
#include "bstrlib.h"

extern void bbcode_destroy_tag_stack(bbcode_container *tag_list){
	for (;tag_list->size>0; tag_list->size--){
	    bdestroy(tag_list->array[tag_list->size-1].tag);
	    bdestroy(tag_list->array[tag_list->size-1].open_tag);
	    bdestroy(tag_list->array[tag_list->size-1].close_tag);
	    bdestroy(tag_list->array[tag_list->size-1].default_arg);
	    bdestroy(tag_list->array[tag_list->size-1].childs);
	    bdestroy(tag_list->array[tag_list->size-1].content_handling_u);
	    bdestroy(tag_list->array[tag_list->size-1].param_handling_u);
	    if (tag_list->array[tag_list->size-1].child_list!=NULL){
	    	free(tag_list->array[tag_list->size-1].child_list);
	    }
	}
	if(tag_list->has_root!=0) {
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].tag);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].open_tag);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].close_tag);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].default_arg);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].childs);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].parents);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].content_handling_u);
	    bdestroy(tag_list->array[BBCODE_MAX_CODES].param_handling_u);
	    if (tag_list->array[BBCODE_MAX_CODES].child_list!=NULL){
	    	free(tag_list->array[BBCODE_MAX_CODES].child_list);
	    }
	    if (tag_list->array[BBCODE_MAX_CODES].parent_list!=NULL){
	    	free(tag_list->array[BBCODE_MAX_CODES].parent_list);
	    }
    }
    free(tag_list);
}
extern bbcode_container *bbcode_create_tag_stack(){
	bbcode_container *tag_list=malloc(sizeof(bbcode_container));
	if (tag_list){
		tag_list->size=0;
		tag_list->has_root=0;
		tag_list->is_ready=0;
	}
	return tag_list;
}
/* Insert BBCode Entry To Table */
extern int bbcode_add_element(char *tag, char type, char flags, char *open_tag, char *close_tag, char *default_arg, char *content_handling_u, char *param_handling_u, char *childs, char *parents, int (*content_handling_func)(bstring content, bstring param, bstring user_func), int (*param_handling_func)(bstring content, bstring param, bstring user_func), bbcode_container *tag_list) {
    /* Checking Free Space */
    if (tag_list->size<BBCODE_MAX_CODES){
	    int pos;
	    if (strcmp(tag,"")){
	    	pos=tag_list->size++;
    	} else if(tag_list->has_root==0) {
    		pos=BBCODE_MAX_CODES;
    		tag_list->has_root=1;
    	} else {
    		return 1;
    	}
	    /* Insert Datas */
	    tag_list->array[pos].type								=	type;
	    tag_list->array[pos].flags								=	flags;
	    tag_list->array[pos].tag									=	bfromcstr(tag);
	    tag_list->array[pos].childs								=	bfromcstr(childs);
	    tag_list->array[pos].parents							=	bfromcstr(parents);
	    tag_list->array[pos].open_tag						=	bfromcstr(open_tag);
	    tag_list->array[pos].close_tag						=	bfromcstr(close_tag);
	    tag_list->array[pos].default_arg						=	bfromcstr(default_arg);
	    tag_list->array[pos].content_handling_u		=	bfromcstr(content_handling_u);
	    tag_list->array[pos].param_handling_u			=	bfromcstr(param_handling_u);
	    tag_list->array[pos].content_handling_func	=	content_handling_func;
	    tag_list->array[pos].param_handling_func	=	param_handling_func;
	    tag_list->array[pos].child_list_size					=	0;
	    tag_list->array[pos].parent_list_size				=	0;
	    tag_list->array[pos].child_list							=	NULL;
	    tag_list->array[pos].parent_list						=	NULL;
	    tag_list->is_ready=0;
	    return 0;
    }
    return 1;
}
extern int bbcode_prepare_tag_list(bbcode_container *tag_list) {
	int i, j, k;
	bstring working_string;
	struct bstrList *bsplited;
	int tmp_stack[BBCODE_MAX_CODES];
	int tmp_stack_size=0;
	int find, not_in_list;
	if (tag_list->is_ready==1){
		return 0;
	} else {
		for(i = 0;i<tag_list->size;i++) {
			if (strcmp( tag_list->array[i].childs->data , "all" ) ){
				if (tag_list->array[i].childs->slen>0 && tag_list->array[i].childs->data[0]=='!'){
					tmp_stack_size=0;
					working_string=bstrcpy(tag_list->array[i].childs);
					bdelete(working_string,0,1);
					bsplited=bsplit (working_string, ',' );
					bdestroy (working_string);
					for (j=0;j<bsplited->qty;j++){
						find = bbcode_get_tag_id ((bsplited->entry[j]),tag_list);
						if (find>=0){
							not_in_list=1;
							for (k=0;k<tmp_stack_size;k++){
								if (find==tmp_stack[k]){
									not_in_list=0;
									break;
								}
							}
							if (not_in_list){
								tmp_stack[k]=find;
								tmp_stack_size++;
							}
						}
					}
					bstrListDestroy(bsplited);
					for (j=0; j<tag_list->size; j++){
						not_in_list=1;
						for (k=0;k<tmp_stack_size;k++){
							if (j==tmp_stack[k]){
								not_in_list=0;
								break;
							}
						}
						if (not_in_list){
							if (tag_list->array[i].child_list==NULL){
								tag_list->array[i].child_list=malloc(BBCODE_MAX_CODES*sizeof(int));
							}
							tag_list->array[i].child_list[tag_list->array[i].child_list_size]=j;
							tag_list->array[i].child_list_size++;
						}
					}
				} else {
					tag_list->array[i].child_list_size=0;
					bsplited=bsplit ( tag_list->array[i].childs, ',' );
					for (j=0;j<bsplited->qty;j++){
						find = bbcode_get_tag_id ((bsplited->entry[j]),tag_list);
						if (find>=0){
							not_in_list=1;
							for (k=0;k<tag_list->array[i].child_list_size;k++){
								if (find==tag_list->array[i].child_list[k]){
									not_in_list=0;
									break;
								}
							}
							if (not_in_list){
								if (tag_list->array[i].child_list==NULL){
									tag_list->array[i].child_list=malloc(BBCODE_MAX_CODES*sizeof(int));
								}
								tag_list->array[i].child_list[tag_list->array[i].child_list_size]=find;
								tag_list->array[i].child_list_size++;
							}
						}
					}
					bstrListDestroy(bsplited);
				}
			} else {
				tag_list->array[i].child_list_size=-1;
			}
			/** parents **/
			if (strcmp( tag_list->array[i].parents->data , "all" ) ){
				tag_list->array[i].parent_list_size=0;
				bsplited=bsplit ( tag_list->array[i].parents, ',' );
				for (j=0;j<bsplited->qty;j++){
					find = bbcode_get_tag_id ((bsplited->entry[j]),tag_list);
					if (find>=0){
						not_in_list=1;
						for (k=0;k<tag_list->array[i].parent_list_size;k++){
							if (find==tag_list->array[i].parent_list[k]){
								not_in_list=0;
								break;
							}
						}
						if (not_in_list){
							if (tag_list->array[i].parent_list==NULL){
								tag_list->array[i].parent_list=malloc(BBCODE_MAX_CODES*sizeof(int));
							}
							tag_list->array[i].parent_list[tag_list->array[i].parent_list_size]=find;
							tag_list->array[i].parent_list_size++;
						}
					}
				}
				bstrListDestroy(bsplited);
			} else {
				tag_list->array[i].parent_list_size=-1;
			}
		}
		if (tag_list->has_root){
			i=BBCODE_MAX_CODES;
			if (strcmp( tag_list->array[i].childs->data , "all" ) ){
				if (tag_list->array[i].childs->slen>0 && tag_list->array[i].childs->data[0]=='!'){
					tmp_stack_size=0;
					working_string=bstrcpy(tag_list->array[i].childs);
					bdelete(working_string,0,1);
					bsplited=bsplit (working_string, ',' );
					bdestroy (working_string);
					for (j=0;j<bsplited->qty;j++){
						find = bbcode_get_tag_id ((bsplited->entry[j]),tag_list);
						if (find>=0){
							not_in_list=1;
							for (k=0;k<tmp_stack_size;k++){
								if (find==tmp_stack[k]){
									not_in_list=0;
									break;
								}
							}
							if (not_in_list){
								tmp_stack[k]=find;
								tmp_stack_size++;
							}
						}
					}
					bstrListDestroy(bsplited);
					for (j=0; j<tag_list->size; j++){
						not_in_list=1;
						for (k=0;k<tmp_stack_size;k++){
							if (j==tmp_stack[k]){
								not_in_list=0;
								break;
							}
						}
						if (not_in_list){
							if (tag_list->array[i].child_list==NULL){
								tag_list->array[i].child_list=malloc(BBCODE_MAX_CODES*sizeof(int));
							}
							tag_list->array[i].child_list[tag_list->array[i].child_list_size]=j;
							tag_list->array[i].child_list_size++;
						}
					}
				} else {
					tag_list->array[i].child_list_size=0;
					bsplited=bsplit ( tag_list->array[i].childs, ',' );
					for (j=0;j<bsplited->qty;j++){
						find = bbcode_get_tag_id ((bsplited->entry[j]),tag_list);
						if (find>=0){
							not_in_list=1;
							for (k=0;k<tag_list->array[i].child_list_size;k++){
								if (find==tag_list->array[i].child_list[k]){
									not_in_list=0;
									break;
								}
							}
							if (not_in_list){
								if (tag_list->array[i].child_list==NULL){
									tag_list->array[i].child_list=malloc(BBCODE_MAX_CODES*sizeof(int));
								}
								tag_list->array[i].child_list[tag_list->array[i].child_list_size]=find;
								tag_list->array[i].child_list_size++;
							}
						}
					}
					bstrListDestroy(bsplited);
				}
			} else {
				tag_list->array[i].child_list_size=-1;
			}
		}
		tag_list->is_ready=1;
		return 0;
	}
	return 1;
}
extern char *bbcode_parse_cstring(char *string, bbcode_container *tag_list) {
	bstring bstr_string=bfromcstr(string);
	char *ret;
	int retval;
	retval=bbcode_parse_string(bstr_string,tag_list);
	if (retval==0){
		ret=bstr2cstr(bstr_string,' ');
		bdestroy(bstr_string);
		return ret;
	} else {
		return NULL;
	}
}
extern int bbcode_parse_string(bstring string, bbcode_container *tag_list) {
	/** Firstly, check if tag_list is ready **/
	bbcode_prepare_tag_list(tag_list);
	/* Working Stacks */
	bbcode_stack work_stack;
	work_stack.size=0;
	int drop_stack[STACK_MAX_SIZE];
	int drop_stack_size=0;
	int opened_stack[STACK_MAX_SIZE];
	int opened_stack_size=0;
	int accepted_tags[BBCODE_MAX_CODES];
	int accepted_tags_size;
	/* End Working Stacks */
	/* Position pointers */
	int next_close=0;
	int next_equal=0;
	int lastfound=0; /* last block stacked for parsing */
	int offset=0; /* main pointer position */
	int tmp=0;
	/* Vairables temporaire de travail */
	int i;
	bbcode_stackLine tmp_stack_line;
	bbcode_stackLine before;
	bstring tmpText;
	/* End Working Stack */
	
	do {
		tmp=bstrchrp(string,'[',offset);
		if (tmp==-1)
		{
			offset=string->slen;
		} else {
			offset=tmp;
			if (*((string->data)+offset+1)=='/'){
				if (next_close<=offset){
					next_close=bstrchrp(string,']',offset+2);
				}
				if (next_close>0){
					tmpText = bmidstr (string, offset+2, next_close-(offset+2));
					tmp_stack_line.tagId=bbcode_get_tag_id(tmpText,tag_list);
					bdestroy(tmpText);
					if (tmp_stack_line.tagId>=0){
						for (i=drop_stack_size;i>0;i--){
							if (drop_stack[i-1]==tmp_stack_line.tagId){
								/* Dropping Tag */
								bdelete(string,offset,next_close-offset+1);
								offset--;
								next_equal=next_close=offset;
								break;
							}
						}
						if(i==0){
							/* On a pas trouvé le tag en drop list */
							/* On vérifie qu'on a un élément ouvert correspondant */
							for (i=opened_stack_size;i>0;i--){
								if (opened_stack[i-1]==tmp_stack_line.tagId){
									break;
								}
							}
							if (i!=0){ /* Tag Ouvert, on le traite */
								if (tag_list->array[opened_stack[opened_stack_size-1]].flags&BBCODE_FLAGS_CDATA_NOT_ALLOWED){
									tmp_stack_line.argument=bfromcstr("");
								} else {
									tmp_stack_line.argument=bmidstr(string,lastfound,offset-lastfound);
								}
								tmp_stack_line.tagId=-1;
								if (work_stack.size>0){
									bbcode_stack_pop(&work_stack,&before);
									if (before.tagId<0){
										before.tagId=-1;
										bconcat(before.argument,tmp_stack_line.argument);
										bbcode_stack_push(&work_stack,before);
										before.tagId=-1;
										before.argument=NULL;
										bdestroy(tmp_stack_line.argument);
									} else {
										bbcode_stack_push(&work_stack,before);
										bbcode_stack_push(&work_stack,tmp_stack_line);
										before.tagId=-1;
										before.argument=NULL;
										tmp_stack_line.tagId=-1;
										tmp_stack_line.argument=NULL;
									}
								} else {
									bbcode_stack_push(&work_stack,tmp_stack_line);
									tmp_stack_line.tagId=-1;
									tmp_stack_line.argument=NULL;
								}
								lastfound=offset=next_close+1;
								/* traitement du close */
								for (i--;i<opened_stack_size;){
									drop_stack[drop_stack_size++]=opened_stack[--opened_stack_size];
									bbcode_close_tag(&work_stack, tag_list);
								}
								--drop_stack_size;
								/* Reccup de l'élément contenu */
								continue;
							}
							/* Tag non ouvert, pas en drop list, on le laisse là */
						} else {
							/* suppression du tag et réorganisation de la liste */
							for (i=i;i<drop_stack_size;i++){
								drop_stack[i-1]=drop_stack[i];
							}
							drop_stack_size--;
							continue;
						}
					}
				} else {
					offset=string->slen+1;
				}
			} else {
				if (next_equal<=offset){
					next_equal=bstrchrp(string,'=',offset);
					if (next_equal==-1){
						next_equal=string->slen+5;
					}
				}
				if (next_close<=offset){
					next_close=bstrchrp(string,']',offset+1);
				}
				if (next_close==-1){
					offset=string->slen+1;
				} else {
	/*** OPENING TAG SECTION ***/
					if (next_equal<next_close){
			/*** Arg Section ***/
						/*** Detection du tag Id ***/
						tmpText = bmidstr (string, offset+1, next_equal-(offset+1));
						tmp_stack_line.tagId=bbcode_get_tag_id(tmpText,tag_list);
						if (tmp_stack_line.tagId>=0){
							if (opened_stack_size>0){
								tmp_stack_line.tagId=bbcode_check_tag_id(tmp_stack_line.tagId,tag_list,opened_stack[opened_stack_size-1]);
							} else if (tag_list->array[tmp_stack_line.tagId].parent_list_size>0) {
								tmp_stack_line.tagId=-1;
							} else if (tag_list->has_root) {
								tmp_stack_line.tagId=bbcode_check_tag_id(tmp_stack_line.tagId,tag_list,BBCODE_MAX_CODES);
							}
						}
						bdestroy(tmpText);
						/*** End Detection du tag Id ***/
						if (tmp_stack_line.tagId>=0){
							/*** NO NESTING BEGIN ***/
							int pos_end_tag=next_close;
							if (tag_list->array[tmp_stack_line.tagId].child_list_size==0){
								bstring close_search;
								close_search=bfromcstr("[/");
								bconcat(close_search,tag_list->array[tmp_stack_line.tagId].tag);
								bconchar(close_search,']');
								pos_end_tag=binstr(string,next_close,close_search);
							}
							/*** NO NESTING END ***/
							/*** PARSE ARG BEGIN ***/
							if (tag_list->array[tmp_stack_line.tagId].flags&BBCODE_FLAGS_ARG_PARSING){
								int pos_open=next_equal-1, pos_close=next_equal;
								while (pos_open<pos_close && pos_close>0){
									pos_open=bstrchrp(string,'[',pos_close+1);
									pos_close=bstrchrp(string,']',pos_close+1);
									int j=0;
								}
								if (pos_close>0){
									next_close=pos_close;
								}
								int i=0;
							}
							/*** PARSE ARG END ***/
							if (tag_list->array[tmp_stack_line.tagId].type==BBCODE_TYPE_ARG || tag_list->array[tmp_stack_line.tagId].type==BBCODE_TYPE_OPTARG){
								if (opened_stack_size<STACK_MAX_SIZE){
									opened_stack[opened_stack_size]=tmp_stack_line.tagId;
									opened_stack_size++;
									tmp_stack_line.argument=bmidstr(string,next_equal+1,next_close-next_equal-1);
									if (tag_list->array[tmp_stack_line.tagId].flags&BBCODE_FLAGS_ARG_PARSING){
										bbcode_parse_string(tmp_stack_line.argument, tag_list);
									}
									if (tag_list->array[opened_stack[opened_stack_size-1]].flags&BBCODE_FLAGS_CDATA_NOT_ALLOWED){
										before.argument=bfromcstr("");
									} else {
										before.argument=bmidstr(string,lastfound,offset-lastfound);
									}
									before.tagId=-1;
									bbcode_stack_push(&work_stack,before);
									bbcode_stack_push(&work_stack,tmp_stack_line);
									if (strcmp(tag_list->array[tmp_stack_line.tagId].childs->data,"")==0){
										offset=pos_end_tag;
										lastfound=next_close+1;
									} else {
										lastfound=offset=next_close+1;
									}
									before.tagId=-1;
									before.argument=NULL;
									tmp_stack_line.tagId=-1;
									tmp_stack_line.argument=NULL;
									continue;
								}
							}
							/* On ignore */
						}
			/*** END Arg Section ***/
					} else {
			/*** No Arg Section ***/
						tmpText = bmidstr (string, offset+1, next_close-(offset+1));
						tmp_stack_line.tagId=bbcode_get_tag_id(tmpText,tag_list);
						if (tmp_stack_line.tagId>=0){
							if (opened_stack_size>0){
								tmp_stack_line.tagId=bbcode_check_tag_id(tmp_stack_line.tagId,tag_list,opened_stack[opened_stack_size-1]);
							} else if (tag_list->array[tmp_stack_line.tagId].parent_list_size>0) {
								tmp_stack_line.tagId=-1;
							} else if (tag_list->has_root) {
								tmp_stack_line.tagId=bbcode_check_tag_id(tmp_stack_line.tagId,tag_list,BBCODE_MAX_CODES);
							}
						}
						bdestroy(tmpText);
						if (tmp_stack_line.tagId>=0){
							/*** NO NESTING BEGIN ***/
							int pos_end_tag=next_close;
							if (tag_list->array[tmp_stack_line.tagId].child_list_size==0){
								bstring close_search;
								close_search=bfromcstr("[/");
								bconcat(close_search,tag_list->array[tmp_stack_line.tagId].tag);
								bconchar(close_search,']');
								pos_end_tag=binstrcaseless (string,next_close,close_search);
							}
							/*** NO NESTING END ***/
							if (tag_list->array[tmp_stack_line.tagId].type==BBCODE_TYPE_NOARG || tag_list->array[tmp_stack_line.tagId].type==BBCODE_TYPE_OPTARG){
								if (opened_stack_size<STACK_MAX_SIZE){
									opened_stack[opened_stack_size]=tmp_stack_line.tagId;
									opened_stack_size++;
									tmp_stack_line.argument=NULL;
									if (tag_list->array[opened_stack[opened_stack_size-1]].flags&BBCODE_FLAGS_CDATA_NOT_ALLOWED){
										before.argument=bfromcstr("");
									} else {
										before.argument=bmidstr(string,lastfound,offset-lastfound);
									}
									before.tagId=-1;
									bbcode_stack_push(&work_stack,before);
									bbcode_stack_push(&work_stack,tmp_stack_line);
									if (strcmp(tag_list->array[tmp_stack_line.tagId].childs->data,"")){
										offset=pos_end_tag;
										lastfound=next_close+1;
									} else {
										lastfound=offset=next_close+1;
									}
									before.tagId=-1;
									before.argument=NULL;
									tmp_stack_line.tagId=-1;
									tmp_stack_line.argument=NULL;
									continue;
								}
							} else if(tag_list->array[tmp_stack_line.tagId].type==BBCODE_TYPE_SINGLE) {
								/* Single */
								breplace(string,offset,next_close-(offset)+1,tag_list->array[tmp_stack_line.tagId].open_tag,' ');
								next_equal=next_close=offset;
							}
							/* On ignore */
						}
					}
			/*** No Arg Section ***/
				}
	/*** END OPENING TAG ***/
			}
		}
		++offset;
	} while (offset < (string->slen));
	/*** Ended Parsing, Closing Remainnig Tags & so on ***/
	tmp_stack_line.argument=bmidstr(string,lastfound,string->slen-lastfound);
	tmp_stack_line.tagId=-1;
	if (work_stack.size>0){
		bbcode_stack_pop(&work_stack,&before);
		if (before.tagId==-1){
			bconcat(before.argument,tmp_stack_line.argument);
			bdestroy(tmp_stack_line.argument);
			bbcode_stack_push(&work_stack,before);
			before.tagId=-1;
			before.argument=NULL;
		} else {
			bbcode_stack_push(&work_stack,before);
			bbcode_stack_push(&work_stack,tmp_stack_line);
			before.tagId=-1;
			before.argument=NULL;
			tmp_stack_line.tagId=-1;
			tmp_stack_line.argument=NULL;
		}
	} else {
		bbcode_stack_push(&work_stack,tmp_stack_line);
		tmp_stack_line.tagId=-1;
		tmp_stack_line.argument=NULL;
	}
	while (work_stack.size>2){
		bbcode_close_tag(&work_stack, tag_list);
		bbcode_stack_pop(&work_stack,&before);
		bbcode_stack_push(&work_stack,before);
		if (before.tagId!=-1){
			before.argument=bfromcstr("");
			before.tagId=-1;
			bbcode_stack_push(&work_stack,before);
			before.tagId=-1;
			before.argument=NULL;
		}
	}
	if (work_stack.size==2){
		bbcode_stack_pop(&work_stack,&before);
		bdestroy(before.argument);
	}
	bbcode_stack_pop(&work_stack,&tmp_stack_line);
	bassign(string,tmp_stack_line.argument);
	bdestroy(tmp_stack_line.argument);
	return 0;
}
int bbcode_check_tag_id(int tag_id, bbcode_container *array, int context){
	int i;
	/** If tag has parent Limitation check it's OK */
	if (array->array[tag_id].parent_list_size>0){
		for (i=0;i<array->array[tag_id].parent_list_size;i++){
			if (array->array[tag_id].parent_list[i]==context){
				return tag_id;
			}
		}
	} else	if (array->array[context].child_list_size==-1) {
		return tag_id;
	} else if (array->array[context].child_list_size==0) {
		return -1;
	} else {
		for (i=0;i<array->array[context].child_list_size;i++) {
			if (array->array[context].child_list[i]==tag_id) {
				return tag_id;
			}
		}
	}
	return -1;
}
int bbcode_get_tag_id(const_bstring tagLabel, bbcode_container *array){
    int j;
    if ((tagLabel->slen)==0){return -1;}
    for (j=0;j<array->size;j++) {
        if (biseqcaseless(tagLabel,array->array[j].tag)) {
            return (j);
        }
    }
   return -1;
}
static int bbcode_close_tag(bbcode_stack *stack, bbcode_container *tag_list){
	bbcode_stackLine content, tag, before;
	bstring work_close, content_bs, arg_bs;
	if (stack->size<3){
		return 1;
	}
	bbcode_stack_pop(stack,&content);
	bbcode_stack_pop(stack,&tag);
	bbcode_stack_pop(stack,&before);
	int tagId=tag.tagId;
	if (content.argument==NULL){
		return 1;
	}
	content_bs=bfromcstr("{CONTENT}");
	arg_bs=bfromcstr("{PARAM}");
	if (tagId!=-1){
		if (before.argument!=NULL){
			bconcat(before.argument,tag_list->array[tagId].open_tag);
		} else {
			before.argument=bstrcpy(tag_list->array[tagId].open_tag);
		}
	}
	if (tag_list->array[tagId].type==BBCODE_TYPE_ARG || tag_list->array[tagId].type==BBCODE_TYPE_OPTARG){
		if (tag.argument==NULL){
			tag.argument=bstrcpy(tag_list->array[tagId].default_arg);
		}
		if (tag_list->array[tagId].param_handling_func!=NULL){
			tag_list->array[tagId].param_handling_func(content.argument, tag.argument, tag_list->array[tagId].param_handling_u);
		}
		if (tag_list->array[tagId].content_handling_func!=NULL){
			tag_list->array[tagId].content_handling_func(content.argument, tag.argument, tag_list->array[tagId].content_handling_u);
		}
		bfindreplace(tag.argument,content_bs,content.argument,0);
		bfindreplace(content.argument,arg_bs,tag.argument,0);
		bfindreplace(before.argument,arg_bs,tag.argument,0);
		work_close = bstrcpy(tag_list->array[tagId].close_tag);
		bfindreplace(work_close,arg_bs,tag.argument,0);
		bdestroy(tag.argument);
	} else {
		work_close = bstrcpy(tag_list->array[tagId].close_tag);
	}
	bfindreplace(before.argument,content_bs,content.argument,0);
	bconcat(before.argument,content.argument);
	bconcat(before.argument,work_close);
	before.tagId=-1;
	bbcode_stack_push(stack, before);
	bdestroy(content.argument);
	content.argument=NULL;
	bdestroy(work_close);
	work_close=NULL;
	bdestroy(arg_bs);
	bdestroy(content_bs);
	do {
		if (bbcode_stack_pop(stack,&content)==0){
			if(bbcode_stack_pop(stack,&before)==0){
				if (before.tagId==-1){
					bconcat(before.argument,content.argument);
					bbcode_stack_push(stack, before);
					bdestroy(content.argument);
					content.argument=NULL;
				} else {
					bbcode_stack_push(stack, before);
					bbcode_stack_push(stack, content);
				}
			} else {
				bbcode_stack_push(stack,content);
				break;
			}
		} else {
			break;
		}
	}while(before.tagId==-1);
	return 0;
}
/** Stack Management **/
static int bbcode_stack_push	(bbcode_stack *stack, bbcode_stackLine stackLine){
	if (stack->size<STACK_MAX_SIZE){
	    ++stack->size;
	    /* Insert Datas */
	    stack->stack[stack->size-1]=stackLine;
	    if(stack->size<STACK_MAX_SIZE){
		    stack->stack[stack->size].argument=NULL;
		    stack->stack[stack->size].tagId=-2;
		}
	    return 0;
	} else {
		return 1;
	}
}
static int bbcode_stack_pop(bbcode_stack *stack, bbcode_stackLine *stackLine){
    if (stack->size>0){
	    --(stack->size);
	    *stackLine=stack->stack[stack->size];
	    stack->stack[stack->size].argument=NULL;
	    stack->stack[stack->size].tagId=-2;
	    return 0;
    }else{
    	stackLine->argument=NULL;
    	stackLine->tagId=-1;
    }
    return 1;
}
