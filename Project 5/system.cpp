#include "system.h"
#include "velocityverlet.h"
#include "lennardjones.h"
#include "statisticssampler.h"
#include "unitconverter.h"
#include "math/random.h"

System::System()
{

}

System::~System()
{
    for(Atom *atom : m_atoms) {
        delete atom;
    }
    m_atoms.clear();
}

void System::applyPeriodicBoundaryConditions(int N) {
    int numberOfAtoms = 4*N*N*N;
    double boxLength = UnitConverter::lengthFromAngstroms(5.26)*N;

    for (int i = 0; i < numberOfAtoms; i++){
        Atom *atom = m_atoms[i];
        for (int dim = 0; dim < 3; dim++){
            if (atom->position(dim) < 0) atom->position(dim) = boxLength - atom->position(dim);
            if (atom->position(dim) > boxLength) atom->position(dim) -= boxLength;
        }
    }

    


    // Read here: http://en.wikipedia.org/wiki/Periodic_boundary_conditions#Practical_implementation:_continuity_and_the_minimum_image_convention
}

void System::removeTotalMomentum(int N) {

    double numberOfAtoms = 4*N*N*N;
    vec3 totalVelocity = vec3(0, 0, 0);
    for (int i = 0; i < numberOfAtoms; i++){
        Atom *atom = m_atoms[i];

        totalVelocity += atom->velocity;
    }

    vec3 dVelocity = totalVelocity/numberOfAtoms;
    totalVelocity = vec3(0,0,0);
    for (int i = 0; i < numberOfAtoms; i++){
        Atom *atom = m_atoms[i];
        atom->velocity -= dVelocity;
        /*
        totalVelocity += atom->velocity;
        atom->force = totalVelocity;
        atom->force.print();
        */

    }
    // Find the total momentum and remove momentum equally on each atom so the total momentum becomes zero.
}

void System::createFCCLattice(int numberOfUnitCellsEachDimension, double latticeConstant, double temperature) {
    // You should implement this function properly. Right now, 100 atoms are created uniformly placed in the system of size (10, 10, 10).
    double b = UnitConverter::lengthFromAngstroms(latticeConstant);
    vec3 unitVector = vec3(b, b, b);
    double T = temperature;
    double N = numberOfUnitCellsEachDimension;

    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            for (int k = 0; k < N; k++){

                Atom *atom1 = new Atom(UnitConverter::massFromSI(6.63352088e-26));
                atom1->position.set(i*b, j*b, k*b);
                atom1->resetVelocityMaxwellian(temperature);
                m_atoms.push_back(atom1);

                Atom *atom2 = new Atom(UnitConverter::massFromSI(6.63352088e-26));
                atom2->position.set((0.5+i)*b, (0.5+j)*b, k*b);
                atom2->resetVelocityMaxwellian(temperature);
                m_atoms.push_back(atom2);

                Atom *atom3 = new Atom(UnitConverter::massFromSI(6.63352088e-26));
                atom3->position.set(i*b, (0.5+j)*b, (0.5+k)*b);
                atom3->resetVelocityMaxwellian(temperature);
                m_atoms.push_back(atom3);

                Atom *atom4 = new Atom(UnitConverter::massFromSI(6.63352088e-26));
                atom4->position.set((0.5+i)*b, j*b, (0.5+k)*b);
                atom4->resetVelocityMaxwellian(temperature);
                m_atoms.push_back(atom4);
            }}}

    setSystemSize(vec3(N*b, N*b, N*b)); // Remember to set the correct system size!
}

void System::calculateForces(int N) {
    for(Atom *atom : m_atoms) {
        atom->resetForce();
    }
    double boxLength = UnitConverter::lengthFromAngstroms(5.26)*N;
    int numberOfAtoms = 4*N*N*N;

    double k = UnitConverter::kb;
    double sigma = UnitConverter::lengthFromAngstroms(3.405);
    double epsilon = UnitConverter::temperatureFromSI(119.8)*k;
    for (int i = 0; i < numberOfAtoms; i++){
        Atom *atom1 = m_atoms[i];
        for (int j = i+1; j < numberOfAtoms; j++){
            Atom *atom2 = m_atoms[j];

            double dx = atom1->position(0) - atom2->position(0);
            double dy = atom1->position(1) - atom2->position(1);
            double dz = atom1->position(2) - atom2->position(2);
            if ((dx) < -boxLength/2.0) dx += boxLength;
            else if ((dx) > boxLength/2.0) dx = boxLength - dx;

            if ((dy) < -boxLength/2.0) dy += boxLength;
            else if ((dy) > boxLength/2.0) dy = boxLength - dz;

            if ((dz) < -boxLength/2.0) dz += boxLength;
            else if ((dz) > boxLength/2.0) dz = boxLength - dz;
            vec3 r_vec = vec3(dx, dy, dz);
            double r = r_vec.length();

            double sigmaPRr = sigma/r;
            double sigmaPRr6 =sigmaPRr*sigmaPRr*sigmaPRr*sigmaPRr*sigmaPRr*sigmaPRr;
            double sigmaPRr12 = sigmaPRr6*sigmaPRr6;
            vec3 force = -24*epsilon*(2*sigmaPRr12/r - sigmaPRr6/r)*r_vec/r;

            atom1->force += force;
            atom2->force -= force;
        }
    }

    }

    //m_potential.calculateForces(*this); // this is a pointer, *this is a reference to this object


void System::step(double dt, int N) {
    m_integrator.integrate(*this, dt, N);
    m_steps++;
    m_time += dt;
}
