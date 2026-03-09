#include "ShapeNavigationAgents.h"
#include "Fop.h"

using namespace std;
namespace agent::shapeNav
{
    void TanhFittedStrategy::decideAction(const std::vector<std::vector<double>> &state,
                                          std::vector<std::vector<double>> &action,
                                          bool iexplore)
    {
   

        // require to measure n_r, n_t, s_np, omg.
        for (int i = 0; i < state.size(); i++)
        {
            // Assuming state contains [n_r, n_t, s_np]
            double n_r, n_t, s_nn, s_np, omg;
            if (mode == 6)
            {
                n_r = state[i][0];
                n_t = state[i][1];
                s_nn = state[i][2];
                s_np = state[i][3];
                omg = state[i][4];
            }

            double Lda;
            if (mode==6) // tanh fitting v2
            {
                double coeff_1 = coeffs[0];
                double coeff_2 = coeffs[1];
                double coeff_31 = coeffs[2];
                double coeff_32 = coeffs[3];
                double coeff_4 = coeffs[4];
                

                if (mode==6){
                    double coeff_7 = coeffs[5];
                    double heaviside = (n_r >= 0) ? 1.0 : 0.0;
                    double sign_snp = (s_np > 0) ? 1.0 : ((s_np < 0) ? -1.0 : 0.0);

                    Lda = tanh(coeff_1 * n_r + coeff_2 * n_t * sign_snp + heaviside * coeff_4 * omg * sign_snp +  coeff_7 * tanh(1 * s_nn));
                }
            }
            // Set the action
            // action[i][0] = 1.0; // Assuming action is a single value for simplicity
            action[i][0] = Lda; // Assuming action is a single value for simplicity
        }
    }
    void TanhFittedStrategy::initialize(const std::string &cfgContent, const Config &config)
    {
        mode = config.Read<int>("mode");
        coeffs = Fop::loadvec1d<double>(config.Read<std::string>("signal.coeffs"));
    }
}