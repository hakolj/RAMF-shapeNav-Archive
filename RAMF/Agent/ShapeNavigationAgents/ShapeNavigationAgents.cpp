#include "ShapeNavigationAgents.h"

namespace agent
{
    namespace shapeNav
    {

        void TheoreticalStrategy::decideAction(const std::vector<std::vector<double>> &state,
                                               std::vector<std::vector<double>> &action,
                                               bool iexplore)
        {
            for (int i = 0; i < state.size(); i++)
            {
                // Assuming state contains [n_r, n_t, s_np]
                double n_r = state[i][0];
                double n_t = state[i][1];
                double s_np = state[i][2];

                // \ve p \cdot \ve e_r
                double p_r = -n_t;

                // Calculate the action based on the theoretical strategy
                double Lda = (p_r > 0 ? 1 : -1) * (s_np > 0 ? 1 : -1);

                // Set the action
                // action[i][0] = 1.0; // Assuming action is a single value for simplicity
                action[i][0] = Lda; // Assuming action is a single value for simplicity
            }
        }

    } // namespace shapeNav

}
