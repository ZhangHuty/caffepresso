#include "caffe_frontend.h"
#include "cnn_layers.h"
#include "debug_control.h"
#include "network_model.h"
#include "caffe_proto_params.h"

// Array of CNN nodes. Each node will contain layer type and a pointer to context of the corresponding layer.
CNN_LYR_NODE_T cnnLayerNodes[NO_DEEP_LAYERS];

void caffe_layer_ctx_init() {
	int lyr, mH, mW, nMaps;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;

	for (lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		// First layer to be provided with input dimensions. Subsequent layer input dimensions are
		// derived from this
		REL_INFO("Reading parameter from layer no %d\n", lyr);
		if(lyr == 0) {
			nMaps = NO_INPUT_MAPS;
			mW = INPUT_IMG_WIDTH;
			mH = INPUT_IMG_HEIGHT;
		} else {
			switch(cnn_param_table[lyr-1].lyrType) {
				case CONV:
					pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[lyr-1].pLyrCtx;
					mH = (pConvCtx->convInfo.mapH + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
					mW = (pConvCtx->convInfo.mapW + 2*pConvCtx->convInfo.pad - pConvCtx->convInfo.K + 1 + pConvCtx->convInfo.stride - 1) / pConvCtx->convInfo.stride;
					nMaps = pConvCtx->convInfo.nOutMaps;
					break;
				case POOL:
					pPoolCtx = (POOL_LYR_CTX_T *) cnnLayerNodes[lyr-1].pLyrCtx;
					mH = (pPoolCtx->poolInfo.mapH + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1) / pPoolCtx->poolInfo.stride;
					mW = (pPoolCtx->poolInfo.mapW + 2*pPoolCtx->poolInfo.pad - pPoolCtx->poolInfo.winSize + 1 + pPoolCtx->poolInfo.stride - 1) / pPoolCtx->poolInfo.stride;
					nMaps = pPoolCtx->poolInfo.nMaps;
					break;	
				case ACT:
					pActCtx = (ACT_LYR_CTX_T *)cnnLayerNodes[lyr-1].pLyrCtx;
					mH = pActCtx->actInfo.mapH;
					mW = pActCtx->actInfo.mapW;
					nMaps = pActCtx->actInfo.nMaps;
					break;
				case INNER_PROD:
					pIpCtx = (IP_LYR_CTX_T *)cnnLayerNodes[lyr-1].pLyrCtx;
					mH = 1;
					mW = pIpCtx->ipInfo.nOutput;
					nMaps = 1;
					break;
				case SOFTMAX:
					// Softmax will be the last layer. Hence no need of this
					pSmaxCtx = (SMAX_LYR_CTX_T *)cnnLayerNodes[lyr-1].pLyrCtx;
					mH = 1;
					mW = pSmaxCtx->nInputs;
					nMaps = 1;
					break;
				default:
					REL_INFO("Unsupported CNN layer\n");
			}
		}
		cnnLayerNodes[lyr].lyrType = cnn_param_table[lyr].lyrType;
		switch(cnn_param_table[lyr].lyrType) {

			case CONV:
				cnnLayerNodes[lyr].pLyrCtx = (CONV_LYR_CTX_T *)malloc(sizeof(CONV_LYR_CTX_T));
				pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				
				pConvCtx->convInfo = (CONV_INFO_T){.mapH = mH,
					.mapW = mW,
					.K = cnn_param_table[lyr].K,
					.stride = cnn_param_table[lyr].stride,
					.pad = cnn_param_table[lyr].pad,
					.nInMaps = nMaps,
					.nOutMaps = cnn_param_table[lyr].nOutMaps, .stride = cnn_param_table[lyr].stride, .pad = cnn_param_table[lyr].pad};
				break;
			case POOL:
				cnnLayerNodes[lyr].pLyrCtx = (POOL_LYR_CTX_T *)malloc(sizeof(POOL_LYR_CTX_T));
				pPoolCtx = (POOL_LYR_CTX_T *) cnnLayerNodes[lyr].pLyrCtx;
				pPoolCtx->poolInfo = (POOL_INFO_T) {.mapH = mH,
					.mapW = mW,
					.nMaps = nMaps,	
					.winSize = cnn_param_table[lyr].winSize,	// to be filled from table
					.stride = cnn_param_table[lyr].stride,
					.pad = cnn_param_table[lyr].pad,
					.poolType = cnn_param_table[lyr].poolType};	// to be filled from table
				break;
			case ACT:
				cnnLayerNodes[lyr].pLyrCtx = (ACT_LYR_CTX_T *)malloc(sizeof(ACT_LYR_CTX_T));
				pActCtx = (ACT_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				pActCtx->actInfo = (ACT_INFO_T) {.nMaps = nMaps,
					.mapH = mH,
					.mapW = mW,
					.actType = cnn_param_table[lyr].actType};	// to be filled from table
				break;
			case INNER_PROD:
				cnnLayerNodes[lyr].pLyrCtx = (IP_LYR_CTX_T *)malloc(sizeof(IP_LYR_CTX_T));
				pIpCtx = (IP_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				pIpCtx->ipInfo = (IP_INFO_T) {.nInput = mH * mW * nMaps,
					.nOutput = cnn_param_table[lyr].nOutputs};	// to  be filled form table
				break;
			case SOFTMAX:
				cnnLayerNodes[lyr].pLyrCtx = (SMAX_LYR_CTX_T *)malloc(sizeof(SMAX_LYR_CTX_T));
				pSmaxCtx = (SMAX_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				pSmaxCtx->nInputs = mH * mW * nMaps;
				break;
			default:
				REL_INFO("Unsupported layer\n");
		}
	}
}

// Internal parameter initialization apart from caffe prototxt config parameters.
void cnn_layer_internal_param_init(void) {
	int lyr, conv_lyr, ip_lyr;
	CONV_LYR_CTX_T *pConvCtx;
	POOL_LYR_CTX_T *pPoolCtx;
	ACT_LYR_CTX_T *pActCtx;
	IP_LYR_CTX_T * pIpCtx;
	SMAX_LYR_CTX_T *pSmaxCtx;
	conv_lyr = 0;
	ip_lyr = 0;
	for( lyr = 0; lyr < NO_DEEP_LAYERS; lyr++) {
		switch(cnnLayerNodes[lyr].lyrType) {

			case CONV:
				pConvCtx = (CONV_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				// TODO: These should come from user after analyzing the dynamic range of the weights and activations after training.
				pConvCtx->convInfo.nKerFractionBits = 11;
				pConvCtx->convInfo.nMapFractionBits = 15;
				// TODO: these modes should be part of some configuration file
				pConvCtx->optType = SCALAR;
				pConvCtx->lyrArithMode = FLOAT_POINT;
				pConvCtx->mapLyt = MAP_ISOLATED;

				// TODO:The optMaps and block width should come from patch size optimizer.
				// Setting them to input map dimensions as of now.
				pConvCtx->blkInfo = (BLK_INFO_T) {.blkH = pConvCtx->convInfo.mapH,
					.blkW = pConvCtx->convInfo.mapW,
					.optMaps = pConvCtx->convInfo.nOutMaps};

				// Pointer to conv weights and biases, taken from the big network model arrays.
				pConvCtx->pFloatKer = conv_w_ptrs[conv_lyr];
				pConvCtx->pFloatBias = conv_b_ptrs[conv_lyr];
				conv_lyr++;
				break;
			case POOL:
				pPoolCtx = (POOL_LYR_CTX_T *) cnnLayerNodes[lyr].pLyrCtx;
				if (pPoolCtx->poolInfo.winSize != 2 || pPoolCtx->poolInfo.stride != 2) {
					pPoolCtx->optType = SCALAR;
				}
				// TODO: these modes should be part of some configuration file
				pPoolCtx->optType = SCALAR;
				pPoolCtx->lyrArithMode = FLOAT_POINT;
				pPoolCtx->mapLyt = MAP_ISOLATED;
				break;
			case ACT:
				pActCtx = (ACT_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				pActCtx->optType = SCALAR;
				pActCtx->lyrArithMode = FLOAT_POINT;
				break;
			case INNER_PROD:
				pIpCtx = (IP_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				// TODO: These should come from user after analyzing the dynamic range of the weights and activations after training.
				pIpCtx->ipInfo.nKerFractionBits = 11;
				pIpCtx->ipInfo.nMapFractionBits = 15;
				// TODO: these modes should be part of some configuration file
				pIpCtx->optType = SCALAR;
				pIpCtx->lyrArithMode = FLOAT_POINT;

				// Pointer to FC layer weights and biases, taken from the big network model arrays.
				pIpCtx->pFloatWeight = ip_w_ptrs[ip_lyr];
				pIpCtx->pFloatBias = ip_b_ptrs[ip_lyr];
				ip_lyr++;
				break;
			case SOFTMAX:
				pSmaxCtx = (SMAX_LYR_CTX_T *)cnnLayerNodes[lyr].pLyrCtx;
				pSmaxCtx->optType = SCALAR;
				pSmaxCtx->lyrArithMode = FLOAT_POINT;
				break;
			default:
				REL_INFO("Invalid layer type");
		}
	}
}