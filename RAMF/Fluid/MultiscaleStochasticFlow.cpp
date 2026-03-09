#include "MultiscaleStochasticFlow.h"
#include "Random.h"
#include <fstream>
#include "Fop.h"

using namespace std;

namespace fluid
{
    void MultiscaleStochasticFlow::initialize(const std::string &path, const Config &config)
    {
        string stpstr = config.Read<string>("main flow indeces", "NULL");
        vector<int> indexlist;
        if (stpstr == "NULL")
        {
            std::cout << "Wrong flow field index list in MultiscaleStochasticFlow" << std::endl;
            // step = config.Read<int>("step");
            // indexlist = vector<int>(1, step);
        }
        else
        {
            stringstream ss;
            int start, end, interv;
            ss << stpstr;
            ss >> start >> end >> interv;
            int curridx = start;
            if (interv == 0)
            {
                cout << "Error: flow field index has zero interv." << endl;
            }
            else
            {
                while (curridx < end + interv)
                {
                    indexlist.push_back(curridx);
                    curridx += interv;
                }
            }
        }

        // _boundaryType = "PPP";

        string flowfieldpath = config.Read<string>("main flow data path");
        flowfieldpath = flowfieldpath + "/";
        mainFlow.LoadData(ms, flowfieldpath, indexlist); // load flow data to datapool
        // making gradients
        mainFlow_gradu.AllocateNew(mainFlow.Size(), ms);
        mainFlow_gradv.AllocateNew(mainFlow.Size(), ms);
        mainFlow_gradw.AllocateNew(mainFlow.Size(), ms);
        for (int i = 0; i < mainFlow.Size(); i++)
        {
            mainFlow.upool[i]->GradientAtCenter(*mainFlow_gradu.upool[i], *mainFlow_gradu.vpool[i], *mainFlow_gradu.wpool[i], 'C');
            mainFlow.vpool[i]->GradientAtCenter(*mainFlow_gradv.upool[i], *mainFlow_gradv.vpool[i], *mainFlow_gradv.wpool[i], 'C');
            mainFlow.wpool[i]->GradientAtCenter(*mainFlow_gradw.upool[i], *mainFlow_gradw.vpool[i], *mainFlow_gradw.wpool[i], 'C');
        }

        string stpstr_noise = config.Read<string>("noise flow indeces", "NULL");
        indexlist.clear();
        if (stpstr_noise == "NULL")
        {
            std::cout << "Wrong flow field index list in MultiscaleStochasticFlow" << std::endl;
            // step = config.Read<int>("step");
            // indexlist = vector<int>(1, step);
        }
        else
        {
            stringstream ss;
            int start, end, interv;
            ss << stpstr_noise;
            ss >> start >> end >> interv;
            int curridx = start;
            if (interv == 0)
            {
                cout << "Error: flow field index has zero interv." << endl;
            }
            else
            {
                while (curridx < end + interv)
                {
                    indexlist.push_back(curridx);
                    curridx += interv;
                }
            }
        }
        string flowfieldpath_noise = config.Read<string>("noise flow data path");
        flowfieldpath_noise = flowfieldpath_noise + "/";
        noiseFlow.LoadData(noiseMs, flowfieldpath_noise, indexlist); // load flow data to datapool
                                                                     // making gradients
        noiseFlow_gradu.AllocateNew(noiseFlow.Size(), noiseMs);
        noiseFlow_gradv.AllocateNew(noiseFlow.Size(), noiseMs);
        noiseFlow_gradw.AllocateNew(noiseFlow.Size(), noiseMs);
        for (int i = 0; i < noiseFlow.Size(); i++)
        {
            noiseFlow.upool[i]->GradientAtCenter(*noiseFlow_gradu.upool[i], *noiseFlow_gradu.vpool[i], *noiseFlow_gradu.wpool[i], 'C');
            noiseFlow.vpool[i]->GradientAtCenter(*noiseFlow_gradv.upool[i], *noiseFlow_gradv.vpool[i], *noiseFlow_gradv.wpool[i], 'C');
            noiseFlow.wpool[i]->GradientAtCenter(*noiseFlow_gradw.upool[i], *noiseFlow_gradw.vpool[i], *noiseFlow_gradw.wpool[i], 'C');
        }

        mfCoeff.resize(mainFlow.Size());
        nfCoeff.resize(noiseFlow.Size());

        tau_mf = config.Read<double>("tau main flow");
        tau_noise = config.Read<double>("tau noise");

        // noise_lscale = config.Read<double>("noise.lscale");
        noise_uscale = config.Read<double>("noise.uscale");
        mainFlow_uscale = config.Read<double>("mainflow.uscale", 1.0);

        return;
    }

    void MultiscaleStochasticFlow::reset()
    {
        // initialize the coefficients, and normalize them
        for (int i = 0; i < mfCoeff.size(); i++)
        {
            mfCoeff[i] = rd::Normal(reFluid);
        }

        for (int i = 0; i < nfCoeff.size(); i++)
        {
            nfCoeff[i] = rd::Normal(reFluid);
        }
        _NormalizeCoeff();
    }

    void MultiscaleStochasticFlow::update(double dt)
    {
        // update the coefficients

        double theta_mf = 1.0 / tau_mf;
        double theta_noise = 1.0 / tau_noise;
        // double sigma = sqrt(2*theta_noise);
        for (int i = 0; i < mfCoeff.size(); i++)
        {
            mfCoeff[i] += rd::ounoise(mfCoeff[i], theta_mf, sqrt(2 * theta_mf / mainFlow.Size()), 0.0, dt, reFluid);
        }
        for (int i = 0; i < nfCoeff.size(); i++)
        {
            nfCoeff[i] += rd::ounoise(nfCoeff[i], theta_noise, sqrt(2 * theta_noise / noiseFlow.Size()), 0.0, dt, reFluid);
        }
        // _NormalizeCoeff();
    }

    void MultiscaleStochasticFlow::_NormalizeCoeff()
    {
        double summf = 0;
        double sumnf = 0;

        for (int i = 0; i < mfCoeff.size(); i++)
            summf += mfCoeff[i] * mfCoeff[i];

        for (int i = 0; i < mfCoeff.size(); i++)
            mfCoeff[i] /= sqrt(summf);

        for (int i = 0; i < nfCoeff.size(); i++)
            sumnf += nfCoeff[i] * nfCoeff[i];

        for (int i = 0; i < nfCoeff.size(); i++)
            nfCoeff[i] /= sqrt(sumnf);
    }

    double readDomainSize(std::string str)
    {
        size_t iPI = str.find("PI");
        stringstream ss;
        double L;
        bool pflag = iPI != string::npos;
        if (pflag)
        {
            str = str.substr(0, iPI);
        }
        ss.str("");
        ss.clear();
        ss << str;
        ss >> L;
        if (pflag)
            L *= M_PI;
        return L;
    }

    std::shared_ptr<MultiscaleStochasticFlow> MultiscaleStochasticFlow::makeInstance(const Config &config)
    {
        string str = config.Read<string>("Mesh Number");
        stringstream ss;
        int Nx, Ny, Nz;
        double Lx, Ly, Lz;
        ss << str;
        ss >> Nx >> Ny >> Nz;

        str = config.Read<string>("DomainX");
        Lx = readDomainSize(str);
        str = config.Read<string>("DomainY");
        Ly = readDomainSize(str);
        str = config.Read<string>("DomainZ");
        Lz = readDomainSize(str);

        shared_ptr<Geometry> geo = make_shared<Geometry_prdXYZ>(Nx, Ny, Nz, Lx, Ly, Lz);
        // geo = new Geometry_prdXYZ(Nx, Ny, Nz, Lx, Ly, Lz);
        geo->InitMeshEdge(); // uniform mesh

        geo->InitMesh();
        geo->InitIndices();
        geo->InitInterval();
        geo->InitWaveNumber();
        Mesh mesh(*geo);

        str = config.Read<string>("noise Mesh Number");
        stringstream ss_noise;
        int noiseNx, noiseNy, noiseNz;
        double noiseLx, noiseLy, noiseLz;
        ss_noise << str;
        ss_noise >> noiseNx >> noiseNy >> noiseNz;

        str = config.Read<string>("noise.DomainX");
        noiseLx = readDomainSize(str);
        str = config.Read<string>("noise.DomainY");
        noiseLy = readDomainSize(str);
        str = config.Read<string>("noise.DomainZ");
        noiseLz = readDomainSize(str);

        shared_ptr<Geometry> noisegeo = make_shared<Geometry_prdXYZ>(noiseNx, noiseNy, noiseNz, noiseLx, noiseLy, noiseLz);
        noisegeo->InitMeshEdge(); // uniform mesh

        noisegeo->InitMesh();
        noisegeo->InitIndices();
        noisegeo->InitInterval();
        noisegeo->InitWaveNumber();
        Mesh noiseMesh(*noisegeo);

        return make_shared<MultiscaleStochasticFlow>(mesh, noiseMesh);
    }

    void MultiscaleStochasticFlow::fluidVelAtPoint(const vectors3d &pos, vectors3d &uf) const
    {

#pragma omp parallel for
        for (int pn = 0; pn < pos.size(); pn++)
        {
            uf[pn] = vec3d::Zero();

            vec3d temppos;
            temppos[0] = fmod(pos[pn][0], ms.Lx);
            temppos[1] = fmod(pos[pn][1], ms.Ly);
            temppos[2] = fmod(pos[pn][2], ms.Lz);
            // cout << temppos << endl;
            temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
            temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
            temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);

            {
                unsigned int fieldnum = mainFlow.Size();
                const Scalar *ufields[fieldnum];
                const Scalar *vfields[fieldnum];
                const Scalar *wfields[fieldnum];
                vectors3d interpResults{fieldnum, vec3d::Zero()};
                for (int j = 0; j < mainFlow.Size(); j++)
                {
                    ufields[j] = mainFlow.upool[j].get();
                    vfields[j] = mainFlow.vpool[j].get();
                    wfields[j] = mainFlow.wpool[j].get();
                }
                interpolater.interp3dMultiFields(temppos, fieldnum, ufields, vfields, wfields, interpResults, FieldStoreType::CCC);

                for (int j = 0; j < mainFlow.Size(); j++)
                {
                    uf[pn] = uf[pn] + interpResults[j] * mfCoeff[j] * mainFlow_uscale;
                }
            }
            if (noiseFlow.Size() > 0)
            {

                // interpolation for noise
                temppos[0] = fmod(pos[pn][0], noiseMs.Lx);
                temppos[1] = fmod(pos[pn][1], noiseMs.Ly);
                temppos[2] = fmod(pos[pn][2], noiseMs.Lz);
                // cout << temppos << endl;
                temppos[0] = fmod(temppos[0] + noiseMs.Lx, noiseMs.Lx);
                temppos[1] = fmod(temppos[1] + noiseMs.Ly, noiseMs.Ly);
                temppos[2] = fmod(temppos[2] + noiseMs.Lz, noiseMs.Lz);

                {
                    unsigned int fieldnum = noiseFlow.Size();
                    const Scalar *ufields[fieldnum];
                    const Scalar *vfields[fieldnum];
                    const Scalar *wfields[fieldnum];
                    vectors3d interpResults{fieldnum, vec3d::Zero()};
                    for (int j = 0; j < noiseFlow.Size(); j++)
                    {
                        ufields[j] = noiseFlow.upool[j].get();
                        vfields[j] = noiseFlow.vpool[j].get();
                        wfields[j] = noiseFlow.wpool[j].get();
                    }
                    interpolater.interp3dMultiFields(temppos, fieldnum, ufields, vfields, wfields, interpResults, FieldStoreType::CCC);

                    for (int j = 0; j < noiseFlow.Size(); j++)
                    {
                        uf[pn] = uf[pn] + interpResults[j] * nfCoeff[j] * noise_uscale;
                    }
                }
            }
        }
    }

    void MultiscaleStochasticFlow::fluidVelGradAtPoint(const vectors3d &pos, vectors3d &grad, int velcomponent) const
    {
        const FlowFieldDataPool *mfgrad, *nfgrad; // pointers to the grad pools for specific direction
        if (velcomponent == 0)
        {
            mfgrad = &mainFlow_gradu;
            nfgrad = &noiseFlow_gradu;
        }
        else if (velcomponent == 1)
        {
            mfgrad = &mainFlow_gradv;
            nfgrad = &noiseFlow_gradv;
        }
        else if (velcomponent == 2)
        {
            mfgrad = &mainFlow_gradw;
            nfgrad = &noiseFlow_gradw;
        }

#pragma omp parallel for
        for (int pn = 0; pn < pos.size(); pn++)
        {
            grad[pn] = vec3d::Zero();

            vec3d temppos;

            // interporlation of main field
            temppos[0] = fmod(pos[pn][0], ms.Lx);
            temppos[1] = fmod(pos[pn][1], ms.Ly);
            temppos[2] = fmod(pos[pn][2], ms.Lz);
            // cout << temppos << endl;
            temppos[0] = fmod(temppos[0] + ms.Lx, ms.Lx);
            temppos[1] = fmod(temppos[1] + ms.Ly, ms.Ly);
            temppos[2] = fmod(temppos[2] + ms.Lz, ms.Lz);
            {
                unsigned int fieldnum = mfgrad->Size();
                const Scalar *ufields[fieldnum];
                const Scalar *vfields[fieldnum];
                const Scalar *wfields[fieldnum];
                vectors3d interpResults{fieldnum, vec3d::Zero()};
                for (int j = 0; j < mfgrad->Size(); j++)
                {
                    ufields[j] = mfgrad->upool[j].get();
                    vfields[j] = mfgrad->vpool[j].get();
                    wfields[j] = mfgrad->wpool[j].get();
                }
                interpolater.interp3dMultiFields(temppos, fieldnum, ufields, vfields, wfields, interpResults, FieldStoreType::CCC);

                for (int j = 0; j < mfgrad->Size(); j++)
                {
                    grad[pn] = grad[pn] + interpResults[j] * mfCoeff[j] * mainFlow_uscale;
                }
            }
            if (noiseFlow.Size() > 0)
            {

                // interpolation for noise
                temppos[0] = fmod(pos[pn][0], noiseMs.Lx);
                temppos[1] = fmod(pos[pn][1], noiseMs.Ly);
                temppos[2] = fmod(pos[pn][2], noiseMs.Lz);
                // cout << temppos << endl;
                temppos[0] = fmod(temppos[0] + noiseMs.Lx, noiseMs.Lx);
                temppos[1] = fmod(temppos[1] + noiseMs.Ly, noiseMs.Ly);
                temppos[2] = fmod(temppos[2] + noiseMs.Lz, noiseMs.Lz);
                {
                    unsigned int fieldnum = nfgrad->Size();
                    const Scalar *ufields[fieldnum];
                    const Scalar *vfields[fieldnum];
                    const Scalar *wfields[fieldnum];
                    vectors3d interpResults{fieldnum, vec3d::Zero()};
                    for (int j = 0; j < nfgrad->Size(); j++)
                    {
                        ufields[j] = nfgrad->upool[j].get();
                        vfields[j] = nfgrad->vpool[j].get();
                        wfields[j] = nfgrad->wpool[j].get();
                    }
                    interpolater.interp3dMultiFields(temppos, fieldnum, ufields, vfields, wfields, interpResults, FieldStoreType::CCC);

                    for (int j = 0; j < nfgrad->Size(); j++)
                    {
                        grad[pn] = grad[pn] + nfCoeff[j] * noise_uscale * interpResults[j];
                    }
                }
            }
        }
    }

    void MultiscaleStochasticFlow::infoAtPoint(const vectors3d &pos, vectors3d &uf, vectors3d &gradu, vectors3d &gradv, vectors3d &gradw) const
    {
        // to be optimized
        fluidVelAtPoint(pos, uf);
        fluidVelGradAtPoint(pos, gradu, 0);
        fluidVelGradAtPoint(pos, gradv, 1);
        fluidVelGradAtPoint(pos, gradw, 2);
    }

    void MultiscaleStochasticFlow::dump(const char *path, int step)
    {
        ofstream os;

        char stepstr[10];
        cout << "dumping fluid field at step: " << step << endl;
        sprintf(stepstr, "%.7i", step);
        string fullpath = string(path) + "/fluidfield" + string(stepstr) + ".txt";
        os.open(fullpath, ios::out | ios::trunc);
        os.precision(8);
        os << scientific;
        for (unsigned i = 0; i < mfCoeff.size(); i++)
        {
            os << mfCoeff[i] << " "; // write main flow coeff
        }
        os << endl;
        for (unsigned i = 0; i < nfCoeff.size(); i++)
        {
            os << nfCoeff[i] << " "; // write noise flow coeff
        }
        os << endl;
        os.close();
    }

    void MultiscaleStochasticFlow::_loadFlow(const char *path, int step)
    {

        char stepstr[10];
        cout << "loading fluid field at step: " << step << endl;
        sprintf(stepstr, "%.7i", step);
        string fullpath = string(path) + "/fluidfield" + string(stepstr) + ".txt";
        ifstream is;
        string line;
        is.open(fullpath);
        if (!is)
            throw std::runtime_error("fluidfield not found: " + fullpath);

        { // main flow coeff
            std::getline(is, line);
            std::istringstream iss(line); // 将读取的行数据存入字符串流
            double value;
            int i = 0;
            while (iss >> value)
            {                         // 读取行中的每个整数
                mfCoeff[i++] = value; // 将整数存入当前行的vector中
            }
        }
        { // noise flow coeff
            std::getline(is, line);
            std::istringstream iss(line); // 将读取的行数据存入字符串流
            double value;
            int i = 0;
            while (iss >> value)
            {                         // 读取行中的每个整数
                nfCoeff[i++] = value; // 将整数存入当前行的vector中
            }
        }
    }
}