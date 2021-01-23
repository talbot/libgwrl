
#include "gwrl/proactor.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void gwpr_src_activity(gwrl * rl, gwrlevt * evt);

gwpr *
gwpr_create(gwrl * rl) {
	gwpr * pr = (gwpr *)gwrl_mem_calloc(1,sizeof(gwpr));
	gwpr_options options = {
		GWPR_MAX_ACCEPT,
		GWPR_WRQUEUE_CACHE_MAX,
		GWPR_IOCP_OVLP_CACHE_MAX,
		GWPR_SYNCHRONOUS_WRITE_MAX_BYTES
	};

	#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!pr) {
            gwrl_err("(Dle3d) calloc error");
			return NULL;
		}
	#endif

	pr->bufctl = gwprbufctl_create();

	#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!pr->bufctl) {
			free(pr);
			return NULL;
		}
	#endif

	gwpr_set_options(pr,&options);
	pr->rl = rl;
	rl->pr = pr;
	return pr;
}

gwprdata *
gwprdata_create() {
	gwprdata * data = _gwprdata(gwrl_mem_calloc(1,sizeof(gwprdata)));
	#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!data) {
            gwrl_err("(Do4Kd) calloc error");
			return NULL;
		}
	#endif

	data->rdbufsize = 512;
	data->rdfilters = NULL;
	data->wrfilters = NULL;

	if(GWPR_FILTERS_MAX > 0) {
		data->rdfilters = gwrl_mem_calloc(1,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
		#ifndef GWRL_HIDE_FROM_COVERAGE
			if(!data->rdfilters) {
				free(data);
				return NULL;
			}
		#endif

		data->wrfilters = gwrl_mem_calloc(1,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
		#ifndef GWRL_HIDE_FROM_COVERAGE
			if(!data->wrfilters) {
				free(data);
				free(data->rdfilters);
				return NULL;
			}
		#endif
	}
	return data;
}

gwrlsrc *
gwpr_set_fd(gwpr * pr, fileid_t fd, void * udata) {
	gwrlsrc_file * fsrc = NULL;
	gwrlsrc * src = gwrl_src_file_create(fd,0,0,NULL);
	#ifndef GWRL_HIDE_FROM_COVERAGE
	while(!src) src = gwrl_src_file_create(fd,0,0,NULL);
	#endif
	fsrc = (gwrlsrc_file *)src;
	src->userdata = udata;
	gwpr_src_add(pr,src);
	return src;
}

gwprbufctl *
gwprbufctl_create() {
	gwprbufctl * ctl = (gwprbufctl *)gwrl_mem_calloc(1,sizeof(gwprbufctl));
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!ctl) {
        gwrl_err("(p4P0R) calloc error");
		return NULL;
	}
	#endif
	return ctl;
}

void
gwprbuf_free(gwprbufctl * bufctl, gwprbuf * buf) {
	free(buf->buf);
	free(buf);
}

void
gwpr_set_options(gwpr * pr, gwpr_options * opts) {
	memcpy(&pr->options,opts,sizeof(pr->options));
}

void
gwpr_src_add(gwpr * pr, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwprdata * data = gwprdata_create();
	#ifndef GWRL_HIDE_FROM_COVERAGE
	while(!data) data = gwprdata_create();
	#endif
	src->flags = GWRL_RD; //read but disabled
	src->callback = gwpr_src_activity;
	fsrc->pdata = data;
	gwrl_src_add(pr->rl,src);
}

void
gwpr_src_add_safely(gwpr * pr, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwprdata * data = gwprdata_create();
	#ifndef GWRL_HIDE_FROM_COVERAGE
	while(!data) data = gwprdata_create();
	#endif
	src->flags = GWRL_RD; //read but disabled
	src->callback = gwpr_src_activity;
	fsrc->pdata = data;
	gwrl_src_add_safely(pr->rl,src);
}

void
gwpr_src_remove(gwpr * pr, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	if(fsrc->pdata) {
		free(fsrc->pdata);
		fsrc->pdata = NULL;
	}
	gwrl_src_remove(pr->rl,src);
}

void
gwpr_src_del(gwpr * pr, gwrlsrc * src) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	if(fsrc->pdata) {
		free(fsrc->pdata);
		fsrc->pdata = NULL;
	}
	gwrl_src_del(pr->rl,src,NULL,true);
}

void
gwpr_filter_add(gwpr * pr, gwrlsrc * src, gwpr_filter_id fid, gwpr_io_cb * fnc) {
	#if GWPR_FILTERS_MAX > 0
		int i = 0;
		bool added = false;
		gwprdata * pdata = _gwprdata(_gwrlsrcf(src)->pdata);
	#else
        gwrl_err("gwper_filter_add, no filter slots available.");
		return;
	#endif

	#if GWPR_FILTERS_MAX > 0
		if(fid == gwpr_rdfilter_id && GWPR_FILTERS_MAX > 0) {
			for(; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->rdfilters[i]) {
					pdata->rdfilters[i] = fnc;
					added = true;
					break;
				}
			}
		} else if(fid == gwpr_wrfilter_id && GWPR_FILTERS_MAX > 0) {
			for(; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->wrfilters[i]) {
					pdata->wrfilters[i] = fnc;
					added = true;
					break;
				}
			}
		}
		if(!added) gwerr("gwper_filter_add, no filter slots available.");
	#endif
}

void
gwpr_filter_reset(gwpr * pr, gwrlsrc * src, gwpr_filter_id fid) {
	#if GWPR_FILTERS_MAX > 0
		gwprdata * pdata = _gwprdata(_gwrlsrcf(src)->pdata);
		if(GWPR_FILTERS_MAX > 0) {
			if(fid == gwpr_rdfilter_id && pdata->rdfilters) {
				bzero(pdata->rdfilters,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
			} else if(fid == gwpr_wrfilter_id && pdata->wrfilters) {
				bzero(pdata->wrfilters,sizeof(gwpr_io_cb *) * GWPR_FILTERS_MAX);
			}
		}
	#endif
}

void
gwpr_filter_call(gwpr * pr, gwrlsrc * src, gwpr_io_info * ioinfo, gwpr_filter_id fid) {
	gwprdata * pdata = _gwrlsrcf(src)->pdata;
	if(fid == gwpr_rdfilter_id) {
		int i = 0;
		if(pdata->rdfilters && pdata->rdfilters[0] != NULL) {
			for(i=0; i<GWPR_FILTERS_MAX; i++) {
				if(!pdata->rdfilters[i]) break;
				pdata->rdfilters[i](pr,ioinfo);
			}
		}
	} else if(fid == gwpr_wrfilter_id) {
		int i = 0;
		for(; i< GWPR_FILTERS_MAX; i++) {
			if(!pdata->wrfilters[i]) break;
			pdata->wrfilters[i](pr,ioinfo);
		}
	}
}

void
gwpr_set_cb(gwpr * pr, gwrlsrc * src, gwpr_cb_id cbid, void * cb) {
	gwrlsrc_file * fsrc = _gwrlsrcf(src);
	gwprdata * data = _gwprdata(fsrc->pdata);
	switch(cbid) {
	case gwpr_error_cb_id:
		data->errorcb = (gwpr_error_cb *)cb;
		break;
	case gwpr_accept_cb_id:
		data->acceptcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_connect_cb_id:
		data->connectcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_closed_cb_id:
		data->closedcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_did_read_cb_id:
		data->didreadcb = (gwpr_io_cb *)cb;
		break;
	case gwpr_did_write_cb_id:
		data->didwritecb = (gwpr_io_cb *)cb;
		break;
	}
}

gwprbuf *
gwpr_buf_get(gwpr * pr, size_t size) {
	gwprbuf * data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));
	#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!data) {
            gwrl_err("(Erl3F) calloc error");
			return NULL;
		}
	#endif

	data->buf = (char *)gwrl_mem_calloc(1,size);
	#ifndef GWRL_HIDE_FROM_COVERAGE
		if(!data->buf) {
            gwrl_err("(RF4Hk) calloc error");
			free(data);
			return NULL;
		}
	#endif

	data->buf = data->buf;
	data->bufsize = size;

	return data;
}

gwprbuf *
gwpr_buf_getp(gwpr * pr, size_t size) {
	gwprbuf * data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));

	#ifndef GWRL_HIDE_FROM_COVERAGE
	while(!data) {
        gwrl_err("(Erl3F) calloc error");
		data = (gwprbuf *)gwrl_mem_calloc(1,sizeof(gwprbuf));
	}
	#endif

	data->buf = (char *)gwrl_mem_calloc(1,size);
	#ifndef GWRL_HIDE_FROM_COVERAGE
		while(!data->buf) {
            gwrl_err("(RF4Hk) calloc error");
			data->buf = (char *)gwrl_mem_calloc(1,size);
		}
	#endif

	data->buf = data->buf;
	data->bufsize = size;

	return data;
}

gwprbuf *
gwpr_buf_get_tagged(gwpr * pr, size_t size, int tag) {
	gwprbuf * buf = gwpr_buf_get(pr,size);
	if(buf) buf->tag = tag;
	return buf;
}

gwprbuf *
gwpr_buf_get_with_data(gwpr * pr, size_t size, char * data, size_t datasize) {
	if(datasize > size) return NULL;
	gwprbuf * buf = gwpr_buf_getp(pr,size);
	memcpy(buf->buf,data,datasize);
	buf->len = datasize;
	return buf;
}

void
gwpr_buf_free(gwpr * pr, gwprbuf * buf) {
	#ifndef GWRL_HIDE_FROM_COVERAGE
	if(!buf) {
		return;
	}
	#endif
	gwprbuf_free(pr->bufctl,buf);
}

#ifdef __cplusplus
}
#endif
