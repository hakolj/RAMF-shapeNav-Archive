#ifndef PPONETWORKS_H
#define PPONETWORKS_H

#include "torch/torch.h"
#include "torch/script.h"

namespace agent
{
    class IPPOActorNet : public torch::nn::Module
    {
    public:
        virtual torch::Tensor forward(torch::Tensor input) = 0;
    };

    class IPPOCriticNet : public torch::nn::Module
    {
    public:
        virtual torch::Tensor forward(torch::Tensor input) = 0;
    };
}
#endif // !PPONETWORKS_H