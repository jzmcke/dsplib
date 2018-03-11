#define RESAMPLE_OK 0

#define RESAMPLE_TYPE_SINC 0

typedef resample resample_s;
typedef struct resample_cfg {
	int fs_out;
	int type;
	int order;
}

int
resample_process(resample p_self
				,float *p_in
				,float *p_out
				,int fs_in
				,int n
				);
	