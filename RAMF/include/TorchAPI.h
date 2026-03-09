// APIs of torch tensors
#pragma once
#ifndef TORCHAPI_H
#define TORCHAPI_H

#include "pch.h"
#include <torch/torch.h>

namespace TorchAPI
{

	inline void TorchVersion()
	{
		std::cout << "PyTorch version: "
				  << TORCH_VERSION_MAJOR << "."
				  << TORCH_VERSION_MINOR << "."
				  << TORCH_VERSION_PATCH << std::endl;
	}

	// template <typename Tvector>
	// torch::Tensor from_vector(const std::vector<std::vector<Tvector>> &vec)
	// {
	// 	int size0 = vec.size();
	// 	int size1 = vec[0].size();
	// 	torch::Tensor tensor = torch::zeros({size0, size1});
	// 	auto accessor = tensor.accessor<float, 2>();
	// 	for (int i = 0; i < size0; i++)
	// 	{
	// 		for (int j = 0; j < size1; j++)
	// 		{
	// 			accessor[i][j] = float(vec[i][j]);
	// 		}
	// 	}
	// 	return tensor;
	// }

	// template <typename Tvector>
	// torch::Tensor from_vector(const std::vector<Tvector> &vec)
	// {
	// 	int size0 = vec.size();
	// 	int size1 = 1;
	// 	torch::Tensor tensor = torch::zeros({size0, size1});
	// 	auto accessor = tensor.accessor<float, 2>();
	// 	for (int i = 0; i < size0; i++)
	// 	{
	// 		accessor[i][0] = float(vec[i]);
	// 	}
	// 	return tensor;
	// }

	template <typename Tvector>
	torch::Tensor from_vector(const std::vector<std::vector<Tvector>> &vec)
	{
		if (vec.empty())
			return torch::empty({0});

		int rows = vec.size();
		int cols = vec[0].size();

		auto options = torch::TensorOptions().dtype(torch::kFloat32);
		torch::Tensor tensor = torch::empty({rows, cols}, options);

		// tensor's raw data pointer
		float *tensor_data_ptr = tensor.data_ptr<float>();

		for (int i = 0; i < rows; i++)
		{
			// Get the target pointer position for the current row
			float *current_row_ptr = tensor_data_ptr + (i * cols);
			const Tvector *source_row_ptr = vec[i].data();

			// Use constexpr if to decide the path at compile time (C++17)
			if constexpr (std::is_same_v<Tvector, float>)
			{
				// Same type, direct memory copy (fastest)
				std::memcpy(current_row_ptr, source_row_ptr, cols * sizeof(float));
			}
			else
			{
				// Different type, use raw pointer conversion (faster than accessor)
				for (int j = 0; j < cols; j++)
				{
					current_row_ptr[j] = static_cast<float>(source_row_ptr[j]);
				}
			}
		}

		return tensor;
	}
	template <typename Tvector>
	torch::Tensor from_vector(const std::vector<Tvector> &vec)
	{
		int rows = vec.size();
		auto options = torch::TensorOptions().dtype(torch::kFloat32);
		torch::Tensor tensor = torch::empty({rows, 1}, options);
		float *tensor_data_ptr = tensor.data_ptr<float>();

		for (int i = 0; i < rows; i++)
		{
			tensor_data_ptr[i] = static_cast<float>(vec[i]);
		}
		return tensor;
	}

	template <typename T>
	std::vector<std::vector<T>> as2Dvector(torch::Tensor tensor)
	{
		torch::Tensor cpuTensor = tensor.to(torch::kCPU);
		int size0 = tensor.size(0);
		int size1 = tensor.size(1);
		std::vector<std::vector<T>> vec(size0, std::vector<T>(size1));
		auto accessor = cpuTensor.accessor<float, 2>();
		for (int i = 0; i < size0; i++)
		{
			for (int j = 0; j < size1; j++)
			{
				vec[i][j] = accessor[i][j];
			}
		}
		return vec;
	}
	template <typename T>
	std::vector<T> as1Dvector(torch::Tensor tensor)
	{
		torch::Tensor cpuTensor = tensor.to(torch::kCPU);
		int size0 = tensor.size(0);
		std::vector<T> vec(size0);
		auto accessor = cpuTensor.accessor<float, 1>();
		for (int i = 0; i < size0; i++)
		{
			vec[i] = accessor[i];
		}
		return vec;
	}

	// https://discuss.pytorch.org/t/how-to-change-learning-rate-with-libtorch-1-5/78037/6
	template <
		typename Optimizer = torch::optim::Adam,
		typename OptimizerOptions = torch::optim::AdamOptions>
	inline auto SetLR(
		Optimizer &optimizer,
		double learningRate)
		-> void
	{
		for (auto &group : optimizer.param_groups())
		{
			if (group.has_options())
			{
				auto &options = static_cast<OptimizerOptions &>(group.options());
				options.lr(learningRate);
			}
		}
	}
};

#endif // !TORCHAPI_H