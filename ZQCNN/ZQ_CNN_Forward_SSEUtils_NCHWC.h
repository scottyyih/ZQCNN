#ifndef _ZQ_CNN_FORWARD_SSE_UTILS_NCHWC_H_
#define _ZQ_CNN_FORWARD_SSE_UTILS_NCHWC_H_
#pragma once
#include "ZQ_CNN_Tensor4D_NCHWC.h"
#include "ZQ_CNN_BBoxUtils.h"
#include "ZQ_CNN_CompileConfig.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <math.h>
#include <omp.h>

namespace ZQ
{
	class ZQ_CNN_Forward_SSEUtils_NCHWC
	{
	public:
		/*
		a = bias - slope * mean / sqrt(var+eps)
		b = slope / sqrt(var+eps)
		value = b * value + a
		*/
		static bool BatchNormScaleBias_Compute_b_a(ZQ_CNN_Tensor4D_NCHWC& b, ZQ_CNN_Tensor4D_NCHWC& a,
			const ZQ_CNN_Tensor4D_NCHWC& mean, const ZQ_CNN_Tensor4D_NCHWC& var, const ZQ_CNN_Tensor4D_NCHWC& scale,
			const ZQ_CNN_Tensor4D_NCHWC& bias, const float eps)
		{
			int C = b.GetC();
			if (C == 0)
				return false;


			if (a.GetC() != C || mean.GetC() != C || var.GetC() != C || scale.GetC() != C || bias.GetC() != C)
				return false;
			float* b_data = b.GetFirstPixelPtr();
			float* a_data = a.GetFirstPixelPtr();
			const float* mean_data = mean.GetFirstPixelPtr();
			const float* var_data = var.GetFirstPixelPtr();
			const float* scale_data = scale.GetFirstPixelPtr();
			const float* bias_data = bias.GetFirstPixelPtr();
			for (int c = 0; c < C; c++)
			{
				b_data[c] = scale_data[c] / sqrt(__max(var_data[c] + eps, 1e-32));
				a_data[c] = bias_data[c] - mean_data[c] * b_data[c];
			}
			return true;
		}

		/*
		a = - slope * mean / sqrt(var+eps)
		b = slope / sqrt(var+eps)
		value = b * value + a
		*/
		static bool BatchNormScale_Compute_b_a(ZQ_CNN_Tensor4D_NCHWC& b, ZQ_CNN_Tensor4D_NCHWC& a,
			const ZQ_CNN_Tensor4D_NCHWC& mean, const ZQ_CNN_Tensor4D_NCHWC& var,
			const ZQ_CNN_Tensor4D_NCHWC& scale, const float eps)
		{
			int C = b.GetC();
			if (C == 0)
				return false;


			if (a.GetC() != C || mean.GetC() != C || var.GetC() != C || scale.GetC() != C)
				return false;
			float* b_data = b.GetFirstPixelPtr();
			float* a_data = a.GetFirstPixelPtr();
			const float* mean_data = mean.GetFirstPixelPtr();
			const float* var_data = var.GetFirstPixelPtr();
			const float* scale_data = scale.GetFirstPixelPtr();
			for (int c = 0; c < C; c++)
			{
				b_data[c] = scale_data[c] / sqrt(__max(var_data[c] + eps, 1e-32));
				a_data[c] = -mean_data[c] * b_data[c];
			}
			return true;
		}

		template<class Tensor4D>
		static bool InnerProductWithBias(Tensor4D& input, const Tensor4D& filters,
			const Tensor4D& bias, Tensor4D& output, void** buffer = 0, __int64* buffer_len = 0)
		{
			return ConvolutionWithBias(input, filters, bias, 1, 1, 1, 1, 0, 0, output, buffer, buffer_len);
		}

		template<class Tensor4D>
		static bool InnerProduct(Tensor4D& input, const Tensor4D& filters,
			Tensor4D& output, void** buffer = 0, __int64* buffer_len = 0)
		{
			return Convolution(input, filters, 1, 1, 1, 1, 0, 0, output, buffer, buffer_len);
		}

		/****************************/

		static bool ConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			const ZQ_CNN_Tensor4D_NCHWC1& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC1& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			const ZQ_CNN_Tensor4D_NCHWC1& bias, const ZQ_CNN_Tensor4D_NCHWC1& slope, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC1& output,
			void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithPReLU(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			const ZQ_CNN_Tensor4D_NCHWC1& slope, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC1& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool Convolution(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC1& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool DepthwiseConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			const ZQ_CNN_Tensor4D_NCHWC1& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC1& output);

		static bool DepthwiseConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			const ZQ_CNN_Tensor4D_NCHWC1& bias, const ZQ_CNN_Tensor4D_NCHWC1& prelu_slope, int strideH, int strideW, int dilation_H, int dilation_W,
			int padH, int padW, ZQ_CNN_Tensor4D_NCHWC1& output);

		static bool DepthwiseConvolution(ZQ_CNN_Tensor4D_NCHWC1& input, const ZQ_CNN_Tensor4D_NCHWC1& filters,
			int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC1& output);

		static void MaxPooling(const ZQ_CNN_Tensor4D_NCHWC1 &input, ZQ_CNN_Tensor4D_NCHWC1 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);

		static void AVGPooling(const ZQ_CNN_Tensor4D_NCHWC1 &input, ZQ_CNN_Tensor4D_NCHWC1 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);

		static bool AddBiasPReLU(ZQ_CNN_Tensor4D_NCHWC1 &input, const ZQ_CNN_Tensor4D_NCHWC1& bias, const ZQ_CNN_Tensor4D_NCHWC1& slope);

		static bool PReLU(ZQ_CNN_Tensor4D_NCHWC1 &input, const ZQ_CNN_Tensor4D_NCHWC1& slope);

		static bool ReLU(ZQ_CNN_Tensor4D_NCHWC1 &input, float slope);

		static bool Softmax(ZQ_CNN_Tensor4D_NCHWC1 &input, int axis);

		/*
		a = bias - slope * mean / sqrt(var+eps)
		b = slope / sqrt(var+eps)
		value = b * value + a
		***OR***
		a = -mean/sqrt(var+eps)
		b = 1/sqrt(var+eps)
		value = b * value + a
		*/
		static bool BatchNorm_b_a(ZQ_CNN_Tensor4D_NCHWC1 &input, const ZQ_CNN_Tensor4D_NCHWC1& b, const ZQ_CNN_Tensor4D_NCHWC1& a);

		static bool Eltwise_Sum(const std::vector<const ZQ_CNN_Tensor4D_NCHWC1*>& input, ZQ_CNN_Tensor4D_NCHWC1& output);

		static bool Eltwise_SumWithWeight(const std::vector<const ZQ_CNN_Tensor4D_NCHWC1*>& input, const std::vector<float>& weight,
			ZQ_CNN_Tensor4D_NCHWC1& output);

		static bool Eltwise_Mul(const std::vector<const ZQ_CNN_Tensor4D_NCHWC1*>& input, ZQ_CNN_Tensor4D_NCHWC1& output);

		static bool Eltwise_Max(const std::vector<const ZQ_CNN_Tensor4D_NCHWC1*>& input, ZQ_CNN_Tensor4D_NCHWC1& output);


#if __ARM_NEON || (ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_SSE)

		static bool ConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			const ZQ_CNN_Tensor4D_NCHWC4& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC4& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			const ZQ_CNN_Tensor4D_NCHWC4& bias, const ZQ_CNN_Tensor4D_NCHWC4& slope, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC4& output,
			void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithPReLU(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			const ZQ_CNN_Tensor4D_NCHWC4& slope, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC4& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool Convolution(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC4& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool DepthwiseConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			const ZQ_CNN_Tensor4D_NCHWC4& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC4& output);

		static bool DepthwiseConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			const ZQ_CNN_Tensor4D_NCHWC4& bias, const ZQ_CNN_Tensor4D_NCHWC4& prelu_slope, int strideH, int strideW, int dilation_H, int dilation_W,
			int padH, int padW, ZQ_CNN_Tensor4D_NCHWC4& output);

		static bool DepthwiseConvolution(ZQ_CNN_Tensor4D_NCHWC4& input, const ZQ_CNN_Tensor4D_NCHWC4& filters,
			int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC4& output);

		static void MaxPooling(const ZQ_CNN_Tensor4D_NCHWC4 &input, ZQ_CNN_Tensor4D_NCHWC4 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);

		static void AVGPooling(const ZQ_CNN_Tensor4D_NCHWC4 &input, ZQ_CNN_Tensor4D_NCHWC4 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);

		static bool AddBiasPReLU(ZQ_CNN_Tensor4D_NCHWC4 &input, const ZQ_CNN_Tensor4D_NCHWC4& bias, const ZQ_CNN_Tensor4D_NCHWC4& slope);

		static bool PReLU(ZQ_CNN_Tensor4D_NCHWC4 &input, const ZQ_CNN_Tensor4D_NCHWC4& slope);

		static bool ReLU(ZQ_CNN_Tensor4D_NCHWC4 &input, float slope);

		static bool Softmax(ZQ_CNN_Tensor4D_NCHWC4 &input, int axis);

		static bool BatchNorm_b_a(ZQ_CNN_Tensor4D_NCHWC4 &input, const ZQ_CNN_Tensor4D_NCHWC4& b, const ZQ_CNN_Tensor4D_NCHWC4& a);

		static bool Eltwise_Sum(const std::vector<const ZQ_CNN_Tensor4D_NCHWC4*>& input, ZQ_CNN_Tensor4D_NCHWC4& output);

		static bool Eltwise_SumWithWeight(const std::vector<const ZQ_CNN_Tensor4D_NCHWC4*>& input, const std::vector<float>& weight,
			ZQ_CNN_Tensor4D_NCHWC4& output);

		static bool Eltwise_Mul(const std::vector<const ZQ_CNN_Tensor4D_NCHWC4*>& input, ZQ_CNN_Tensor4D_NCHWC4& output);

		static bool Eltwise_Max(const std::vector<const ZQ_CNN_Tensor4D_NCHWC4*>& input, ZQ_CNN_Tensor4D_NCHWC4& output);

#endif //__ARM_NEON || (ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_SSE)

#if ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_AVX

		static bool ConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			const ZQ_CNN_Tensor4D_NCHWC8& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC8& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			const ZQ_CNN_Tensor4D_NCHWC8& bias, const ZQ_CNN_Tensor4D_NCHWC8& slope, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC8& output,
			void** buffer = 0, __int64* buffer_len = 0);

		static bool ConvolutionWithPReLU(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			const ZQ_CNN_Tensor4D_NCHWC8& slope, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC8& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool Convolution(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters, int strideH, int strideW,
			int dilation_H, int dilation_W, int padH, int padW,
			ZQ_CNN_Tensor4D_NCHWC8& output, void** buffer = 0, __int64* buffer_len = 0);

		static bool DepthwiseConvolutionWithBias(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			const ZQ_CNN_Tensor4D_NCHWC8& bias, int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC8& output);

		static bool DepthwiseConvolutionWithBiasPReLU(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			const ZQ_CNN_Tensor4D_NCHWC8& bias, const ZQ_CNN_Tensor4D_NCHWC8& prelu_slope, int strideH, int strideW, int dilation_H, int dilation_W,
			int padH, int padW, ZQ_CNN_Tensor4D_NCHWC8& output);

		static bool DepthwiseConvolution(ZQ_CNN_Tensor4D_NCHWC8& input, const ZQ_CNN_Tensor4D_NCHWC8& filters,
			int strideH, int strideW, int dilation_H, int dilation_W, int padH, int padW, ZQ_CNN_Tensor4D_NCHWC8& output);

		static void MaxPooling(const ZQ_CNN_Tensor4D_NCHWC8 &input, ZQ_CNN_Tensor4D_NCHWC8 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);
		
		static void AVGPooling(const ZQ_CNN_Tensor4D_NCHWC8 &input, ZQ_CNN_Tensor4D_NCHWC8 &output, int kernel_H, int kernel_W,
			int stride_H, int stride_W, bool global_pool);
		
		static bool AddBiasPReLU(ZQ_CNN_Tensor4D_NCHWC8 &input, const ZQ_CNN_Tensor4D_NCHWC8& bias, const ZQ_CNN_Tensor4D_NCHWC8& slope);
		
		static bool PReLU(ZQ_CNN_Tensor4D_NCHWC8 &input, const ZQ_CNN_Tensor4D_NCHWC8& slope);
		
		static bool ReLU(ZQ_CNN_Tensor4D_NCHWC8 &input, float slope);
		
		static bool Softmax(ZQ_CNN_Tensor4D_NCHWC8 &input, int axis);

		static bool BatchNorm_b_a(ZQ_CNN_Tensor4D_NCHWC8 &input, const ZQ_CNN_Tensor4D_NCHWC8& b, const ZQ_CNN_Tensor4D_NCHWC8& a);

		static bool Eltwise_Sum(const std::vector<const ZQ_CNN_Tensor4D_NCHWC8*>& input, ZQ_CNN_Tensor4D_NCHWC8& output);

		static bool Eltwise_SumWithWeight(const std::vector<const ZQ_CNN_Tensor4D_NCHWC8*>& input, const std::vector<float>& weight,
			ZQ_CNN_Tensor4D_NCHWC8& output);

		static bool Eltwise_Mul(const std::vector<const ZQ_CNN_Tensor4D_NCHWC8*>& input, ZQ_CNN_Tensor4D_NCHWC8& output);
		
		static bool Eltwise_Max(const std::vector<const ZQ_CNN_Tensor4D_NCHWC8*>& input, ZQ_CNN_Tensor4D_NCHWC8& output);

#endif //ZQ_CNN_USE_SSETYPE >= ZQ_CNN_SSETYPE_AVX
	};

}
#endif
