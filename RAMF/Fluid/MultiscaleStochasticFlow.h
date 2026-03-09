#ifndef MULTISCALESTOCHASTICFLOW_H
#define MULTISCALESTOCHASTICFLOW_H

#include "Fluid.h"
#include "Interpolation.h"
#include "Scalar.h"
#include "FlowFieldDataPool.h"

namespace fluid
{

    class MultiscaleStochasticFlow : public Fluid,
                                     public InfoAtPointAble,
                                     public FluidVelGradAtPointAble,
                                     public FluidVelAtPointAble
    {

    public:
        Mesh ms;
        int Nx, Ny, Nz;
        double Lx, Ly, Lz; // domain size

        Mesh noiseMs; // mesh for noise
        int noiseNx, noiseNy, noiseNz;
        double noiseLx, noiseLy, noiseLz; // domain size
        std::vector<double> mfCoeff;      // coefficients for main flow superposition
        std::vector<double> nfCoeff;      // coefficients for noise flow superposition
        MultiscaleStochasticFlow(const Mesh &ms, const Mesh &noiseMs) : mainFlow(), noiseFlow(), ms(ms),
                                                                        Nx(ms.Nx), Ny(ms.Ny), Nz(ms.Nz), Lx(ms.Lx), Ly(ms.Ly), Lz(ms.Lz),
                                                                        noiseMs(noiseMs),
                                                                        noiseNx(noiseMs.Nx), noiseNy(noiseMs.Ny), noiseNz(noiseMs.Nz), noiseLx(noiseMs.Lx),
                                                                        noiseLy(noiseMs.Ly), noiseLz(noiseMs.Lz),
                                                                        interpolater() {
                                                                            // interpolater.interpCoef(ms.Lx / (ms.Nx - 1), ms.Ly / (ms.Ny - 1), ms.Lz / (ms.Nz - 1));
                                                                        };
        ~MultiscaleStochasticFlow() {};

        virtual void initialize(const std::string &path, const Config &config);
        virtual void reset();
        static std::shared_ptr<MultiscaleStochasticFlow> makeInstance(const Config &config);
        // virtual void infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const;
        // virtual void fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const;
        // virtual void fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const;
        virtual void update(double dt);
        virtual std::string boundaryType() { return "PPP"; }
        void _loadFlow(const char *path, int step);
        virtual void dump(const char *path, int step); // dump fluid info

        inline virtual void getDomainSize(double &Lx, double &Ly, double &Lz)
        {
            Lx = this->Lx;
            Ly = this->Ly;
            Lz = this->Lz;
        }
        inline virtual void getDomainBound(double &xmin, double &xmax, double &ymin, double &ymax, double &zmin, double &zmax)
        {
            xmin = 0;
            xmax = Lx;
            ymin = 0;
            ymax = Ly;
            zmin = 0;
            zmax = Lz;
        }

        void fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const;
        void fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const;
        void infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const;

    protected:
        FlowFieldDataPool mainFlow;
        FlowFieldDataPool noiseFlow;
        FlowFieldDataPool mainFlow_gradu;
        FlowFieldDataPool mainFlow_gradv;
        FlowFieldDataPool mainFlow_gradw;
        FlowFieldDataPool noiseFlow_gradu;
        FlowFieldDataPool noiseFlow_gradv;
        FlowFieldDataPool noiseFlow_gradw;

        double tau_mf;    // correlation time for main flow
        double tau_noise; // correlation time for noise flow

        void _NormalizeCoeff();
        Lag2nd3D interpolater;

        double mainFlow_uscale; // the velocity scale for the main flow: v_main = mainFlow_uscale * vdata
        double noise_uscale;    // the velocity scale for the noise: v_noise = noise_uscale * vdata
        // double noise_lscale; // the length scale for the noise: L_noise = noise_lscale * Lx
    };
}

#endif