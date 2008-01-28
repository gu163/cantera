/**
 * @file vcs_VolPhase.cpp
 */

/* $Id$ */

/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
#include "vcs_VolPhase.h"
#include "vcs_internal.h"
#include "vcs_SpeciesProperties.h"
#include "vcs_species_thermo.h"

#include "ThermoPhase.h"


#include <cstdio>
#include <cstdlib>

namespace VCSnonideal {

/*****************************************************************************
 * 
 *  vcs_VolPhase():
 *
 *    Constructor for the VolPhase object.
 */
vcs_VolPhase::vcs_VolPhase() :
  VP_ID(-1),
  Domain_ID(-1),
  SingleSpecies(true),
  GasPhase(false), 
  LiqPhase(false), 
  EqnState(VCS_EOS_CONSTANT),
  nElemConstraints(0),
  ChargeNeutralityElement(-1),
  ElGlobalIndex(0),
  NVolSpecies(0),
  TMolesInert(0.0),
  ActivityConvention(0),
  Existence(0),
  IndexSpecialSpecies(-1),
  Activity_Coeff_Model(VCS_AC_CONSTANT),
  Activity_Coeff_Params(0),
  IndSpecies(0),
  IndSpeciesContig(true),
  UseCanteraCalls(false),
  m_VCS_UnitsFormat(VCS_UNITS_MKS),
  TP_ptr(0),
  TMoles(0.0),
  Vol(0.0),
  m_phi(0.0),
  m_UpToDate_AC(false),
  m_UpToDate_VolStar(false),
  m_UpToDate_VolPM(false),
  m_UpToDate_GStar(false),
  Temp(273.15),
  Pres(1.01325E5),
  RefPres(1.01325E5)
{
}

/*
 * 
 *  ~vcs_VolPhase():
 *
 *   Destructor for the VolPhase object.
 */
vcs_VolPhase::~vcs_VolPhase() {
  for (int k = 0; k < NVolSpecies; k++) {
    vcs_SpeciesProperties *sp = ListSpeciesPtr[k];
    delete sp;
    sp = 0;
  }
}

/*
 * 
 *  Copy Constructor():
 *
 *  Objects that are owned by this object are deep copied here, except
 *  for the ThermoPhase object.
 *  The assignment operator does most of the work.
 */
vcs_VolPhase::vcs_VolPhase(const vcs_VolPhase& b) :
  VP_ID(b.VP_ID),
  Domain_ID(b.Domain_ID),
  SingleSpecies(b.SingleSpecies),
  GasPhase(b.GasPhase),
  LiqPhase(b.LiqPhase),
  EqnState(b.EqnState),
  nElemConstraints(b.nElemConstraints),
  ChargeNeutralityElement(b.ChargeNeutralityElement),
  NVolSpecies(b.NVolSpecies),
  TMolesInert(b.TMolesInert),
  ActivityConvention(b.ActivityConvention),
  Existence(b.Existence),
  IndexSpecialSpecies(b.IndexSpecialSpecies),
  Activity_Coeff_Model(b.Activity_Coeff_Model),
  Activity_Coeff_Params(b.Activity_Coeff_Params),
  IndSpeciesContig(b.IndSpeciesContig),
  UseCanteraCalls(b.UseCanteraCalls),
  m_VCS_UnitsFormat(b.m_VCS_UnitsFormat),
  TP_ptr(b.TP_ptr),
  TMoles(b.TMoles),
  m_phiVarIndex(-1),
  Vol(b.Vol),
  m_phi(b.m_phi),
  m_UpToDate_AC(false),
  m_UpToDate_VolStar(false),
  m_UpToDate_VolPM(false),
  m_UpToDate_GStar(false),
  Temp(b.Temp),
  Pres(b.Pres)
{
  /*
   * Call the Assignment operator to do the heavy
   * lifting.
   */
  *this = b;
}

/*****************************************************************************
 * Assignment operator()
 *
 *   (note, this is used, so keep it current!)
 */
vcs_VolPhase& vcs_VolPhase::operator=(const vcs_VolPhase& b)
{
  int k;
  if (&b != this) {
    int old_num = NVolSpecies;

    VP_ID               = b.VP_ID;
    Domain_ID           = b.Domain_ID;
    SingleSpecies       = b.SingleSpecies;
    GasPhase            = b.GasPhase;
    LiqPhase            = b.LiqPhase;
    EqnState            = b.EqnState;
 
    NVolSpecies         = b.NVolSpecies;
    nElemConstraints    = b.nElemConstraints;
    ChargeNeutralityElement = b.ChargeNeutralityElement;


    ElName.resize(b.nElemConstraints);
    for (int e = 0; e < b.nElemConstraints; e++) {
      ElName[e] = b.ElName[e];
    }
 
    ElActive = b.ElActive;
    m_elType = b.m_elType;
  
    FormulaMatrix.resize(nElemConstraints, NVolSpecies, 0.0);
    for (int e = 0; e < nElemConstraints; e++) {
      for (int k = 0; k < NVolSpecies; k++) {
	FormulaMatrix[e][k] = b.FormulaMatrix[e][k];
      }
    }

    SpeciesUnknownType = b.SpeciesUnknownType;
    ElGlobalIndex = b.ElGlobalIndex;
    NVolSpecies         = b.NVolSpecies;
    PhaseName           = b.PhaseName;
    TMolesInert         = b.TMolesInert;
    ActivityConvention  = b.ActivityConvention;
    Existence           = b.Existence;
    IndexSpecialSpecies = b.IndexSpecialSpecies;
    Activity_Coeff_Model = b.Activity_Coeff_Model;

    /*
     * Do a shallow copy because we haven' figured this out.
     */
    Activity_Coeff_Params = b.Activity_Coeff_Params;
    IndSpecies = b.IndSpecies;
    IndSpeciesContig = b.IndSpeciesContig;

    for (k = 0; k < old_num; k++) {
      if ( ListSpeciesPtr[k]) {
	delete  ListSpeciesPtr[k];
	ListSpeciesPtr[k] = 0;
      }
    }
    ListSpeciesPtr.resize(NVolSpecies, 0);
    for (k = 0; k < NVolSpecies; k++) {
      ListSpeciesPtr[k] = 
	new vcs_SpeciesProperties(*(b.ListSpeciesPtr[k]));
    }

    UseCanteraCalls     = b.UseCanteraCalls;
    m_VCS_UnitsFormat   = b.m_VCS_UnitsFormat;
    /*
     * Do a shallow copy of the ThermoPhase object pointer.
     * We don't duplicate the object.
     *  Um, there is no reason we couldn't do a 
     *  duplicateMyselfAsThermoPhase() call here. This will
     *  have to be looked into.
     */
    TP_ptr              = b.TP_ptr;
    TMoles              = b.TMoles;
 
    Xmol = b.Xmol;

    m_phi               = b.m_phi;
    m_phiVarIndex       = b.m_phiVarIndex;
 
    SS0ChemicalPotential = b.SS0ChemicalPotential;
    StarChemicalPotential = b.StarChemicalPotential;

    StarMolarVol = b.StarMolarVol;
    PartialMolarVol = b.PartialMolarVol; 
    ActCoeff = b.ActCoeff;

    dLnActCoeffdMolNumber = b.dLnActCoeffdMolNumber;

    m_UpToDate_AC         = false;
    m_UpToDate_VolStar    = false;
    m_UpToDate_VolPM      = false;
    m_UpToDate_GStar      = false;
    Temp                = b.Temp;
    Pres                = b.Pres;
    setState_TP(Temp, Pres);
    _updateMoleFractionDependencies();
  }
  return *this;
}


void vcs_VolPhase::resize(int phaseNum, int nspecies, const char *phaseName,
			  double molesInert) {
  if (nspecies <= 0) {
    plogf("nspecies Error\n");
    exit(-1);
  }
  if (phaseNum < 0) {
    plogf("phaseNum should be greater than 0\n");
    exit(-1);
  }

  TMolesInert = molesInert;
  if (TMolesInert > 0.0) {
    Existence = 2;
  } 

  m_phi = 0.0;
  m_phiVarIndex = -1;

  if (phaseNum == VP_ID) {
    if (strcmp(PhaseName.c_str(), phaseName)) {
      plogf("Strings are different: %s %s :unknown situation\n",
	     PhaseName.c_str(), phaseName);
      exit(-1);
    }
  } else {
    VP_ID = phaseNum;
    if (!phaseName) {
      char itmp[40];
      sprintf(itmp, "Phase_%d", VP_ID);
      PhaseName = itmp;
    } else {
      PhaseName = phaseName;
    }
  }
 if (nspecies > 1) {
    SingleSpecies = false;
 } else {
   SingleSpecies = true;
 }

  if (NVolSpecies == nspecies) {
    return;
  }
 
  NVolSpecies = nspecies;
  if (nspecies > 1) {
    SingleSpecies = false;
  }

  IndSpecies.resize(nspecies,-1);

  if ((int) ListSpeciesPtr.size() >=  NVolSpecies) {
    for (int i = 0; i < NVolSpecies; i++) {
      if (ListSpeciesPtr[i]) {
	delete ListSpeciesPtr[i]; 
	ListSpeciesPtr[i] = 0;
      }
    }
  }
  ListSpeciesPtr.resize(nspecies, 0);
  for (int i = 0; i < nspecies; i++) {
    ListSpeciesPtr[i] = new vcs_SpeciesProperties(phaseNum, i, this);
  }

  Xmol.resize(nspecies, 0.0);
  for (int i = 0; i < nspecies; i++) {
    Xmol[i] = 1.0/nspecies;
  }

  SS0ChemicalPotential.resize(nspecies, -1.0);
  StarChemicalPotential.resize(nspecies, -1.0);
  StarMolarVol.resize(nspecies, -1.0);
  PartialMolarVol.resize(nspecies, -1.0);
  ActCoeff.resize(nspecies, 1.0);
  dLnActCoeffdMolNumber.resize(nspecies, nspecies, 0.0);
 

  SpeciesUnknownType.resize(nspecies, VCS_SPECIES_TYPE_MOLNUM);
  m_UpToDate_AC         = false;
  m_UpToDate_VolStar    = false;
  m_UpToDate_VolPM      = false;
  m_UpToDate_GStar      = false;
}


//! Evaluate activity coefficients
/*!
 *   We carry out a calculation whenever UpTODate_AC is false. Specifically
 *   whenever a phase goes zero, we do not carry out calculations on it.
 */
void vcs_VolPhase::evaluateActCoeff() const {
  char yo[] = "cpc_eval_ac ";
  if (m_UpToDate_AC == true) return;
  if (UseCanteraCalls) {
    TP_ptr->getActivityCoefficients(VCS_DATA_PTR(ActCoeff));
  } else {
    switch (Activity_Coeff_Model) {
    case VCS_AC_CONSTANT:
      /*
       * Don't need to do anything since ActCoeff[] is initialized to
       * the value of one, and never changed for this model.
       */
      break;
    default:
      plogf("%sERROR: unknown model\n", yo);
      exit(-1);
    }
  }
  m_UpToDate_AC = true;
}

/******************************************************************************
 *
 * Evaluate one activity coefficients.
 *
 *   return one activity coefficient. Have to recalculate them all to get
 *   one.
 */
double vcs_VolPhase::AC_calc_one(int kspec) const {
  evaluateActCoeff();
  return(ActCoeff[kspec]);
}

// Gibbs free energy calculation at a temperature for the reference state
// of each species
/*
 *  @param TKelvin temperature
 */
void vcs_VolPhase::G0_calc(double tkelvin) {
  bool lsame = false;
  if (Temp == tkelvin) {
    lsame = true;
  }

  bool doit = !lsame;
  setState_TP(tkelvin, Pres);
  if (SS0ChemicalPotential[0] == -1) doit = true;
  if (doit) {
    if (UseCanteraCalls) {
      TP_ptr->getGibbs_ref(VCS_DATA_PTR(SS0ChemicalPotential));
    } else {
      double R = vcsUtil_gasConstant(m_VCS_UnitsFormat);
      for (int k = 0; k < NVolSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	SS0ChemicalPotential[k] =
	  R * (sTherm->G0_R_calc(kglob, tkelvin));
      }
    }
  }
}

// Gibbs free energy calculation at a temperature for the reference state
// of a species, return a value for one species
/*
 *  @param kspec   species index
 *  @param TKelvin temperature
 *
 *  @return return value of the gibbs free energy
 */
double vcs_VolPhase::G0_calc_one(int kspec, double tkelvin) {
  G0_calc(tkelvin);
  return SS0ChemicalPotential[kspec];
}

// Gibbs free energy calculation for standard states
/*
 * Calculate the Gibbs free energies for the standard states
 * The results are held internally within the object.
 *
 * @param TKelvin Current temperature
 * @param pres    Current pressure
 */
void vcs_VolPhase::GStar_calc(double tkelvin, double pres) {
  setState_TP(tkelvin, pres);
  if (!m_UpToDate_GStar) {
    if (UseCanteraCalls) {
      TP_ptr->getStandardChemPotentials(VCS_DATA_PTR(StarChemicalPotential));
    } else {
      double R = vcsUtil_gasConstant(m_VCS_UnitsFormat);
      for (int k = 0; k < NVolSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarChemicalPotential[k] =
	  R * (sTherm->GStar_R_calc(kglob, tkelvin, pres));
      }
    }
    m_UpToDate_GStar = true;
  }
}

// Gibbs free energy calculation for standard state of one species
/*
 * Calculate the Gibbs free energies for the standard state
 * of the kth species.
 * The results are held internally within the object.
 * The kth species standard state G is returned
 *
 * @param kspec   Species number (within the phase)
 * @param TKelvin Current temperature
 * @param pres    Current pressure
 *
 * @return Gstar[kspec] returns the gibbs free energy for the
 *         standard state of the kth species.
 */
double vcs_VolPhase::GStar_calc_one(int kspec, double tkelvin,
				    double pres) {
  GStar_calc(tkelvin, pres);
  return StarChemicalPotential[kspec];
}

// Set the moles within the phase
/*
 *  This function takes as input the mole numbers in vcs format, and
 *  then updates this object with their values. This is essentially
 *  a gather routine.
 *
 *  
 *  @param molesSpeciesVCS  array of mole numbers. Note, the indecises for species in 
 *            this array may not be contiguous. IndSpecies[] is needed
 *            to gather the species into the local contiguous vector
 *            format. 
 */
void vcs_VolPhase::setMolesFromVCS(const double * const molesSpeciesVCS) {
  int kglob;
  double tmp;
  TMoles = TMolesInert;
  for (int k = 0; k < NVolSpecies; k++) {
    if (SpeciesUnknownType[k] != VCS_SPECIES_TYPE_INTERFACIALVOLTAGE) {
      kglob = IndSpecies[k];
      tmp = MAX(0.0, molesSpeciesVCS[kglob]);
      Xmol[k] = tmp;
      TMoles += tmp;
    }
  }
  if (TMoles > 0.0) {
    for (int k = 0; k < NVolSpecies; k++) {
      Xmol[k] /= TMoles;
    }
    Existence = 1;
  } else {
    // This is where we will start to store a better approximation 
    // for the mole fractions, when the phase doesn't exist.
    // This is currently unimplemented.
    for (int k = 0; k < NVolSpecies; k++) {
      Xmol[k] = 1.0 / NVolSpecies;
    }
    Existence = 0;
  }
  /*
   * Update the electric potential if it is a solution variable
   * in the equation system
   */
  if (m_phiVarIndex >= 0) {
    kglob = IndSpecies[m_phiVarIndex];
    if (NVolSpecies == 1) {
      Xmol[m_phiVarIndex] = 1.0;
    } else {
      Xmol[m_phiVarIndex] = 0.0;
    }
    double phi = molesSpeciesVCS[kglob];
    setElectricPotential(phi);
    if (NVolSpecies == 1) {
      Existence = 1;
    }
  }
  _updateMoleFractionDependencies();
  if (TMolesInert > 0.0) {
    Existence = 2;
  }
}

// Set the mole fractions from a conventional mole fraction vector
/*
 *
 * @param xmol Value of the mole fractions for the species
 *             in the phase. These are contiguous. 
 */
void vcs_VolPhase::setMoleFractions(const double * const xmol) {
  double sum = -1.0;
  for (int k = 0; k < NVolSpecies; k++) {
      Xmol[k] = xmol[k];
      sum+= xmol[k];
  }
  if (fabs(sum) > 1.0E-13) {
    for (int k = 0; k < NVolSpecies; k++) {
      Xmol[k] /= sum;
    }
  }
  _updateMoleFractionDependencies();
}

// Updates the mole fractions in subobjects
/*
 *  Whenever the mole fractions change, this routine
 *  should be called.
 */
void vcs_VolPhase::_updateMoleFractionDependencies() {
  if (UseCanteraCalls) {
    if (TP_ptr) {
      TP_ptr->setState_PX(Pres, VCS_DATA_PTR(Xmol));
    }
  }
  m_UpToDate_AC    = false;
  m_UpToDate_VolPM = false;
}

// Return a const reference to the mole fraction vector in the phase
const std::vector<double> & vcs_VolPhase::moleFractions() const {
  return Xmol;
}


//! Set the moles within the phase
/*!
 *  This function takes as input the mole numbers in vcs format, and
 *  then updates this object with their values. This is essentially
 *  a gather routine.
 *
 *  
 *  @param molesSpeciesVCS  array of mole numbers. Note, the indecises for species in 
 *            this array may not be contiguous. IndSpecies[] is needed
 *            to gather the species into the local contiguous vector
 *            format. 
 */
void vcs_VolPhase::setMolesFromVCSCheck(const double * const molesSpeciesVCS, 
					const double * const TPhMoles,
					int iphase) {
  setMolesFromVCS(molesSpeciesVCS);
  /*
   * Check for consistency with TPhMoles[]
   */
  double Tcheck = TPhMoles[VP_ID];
  if (Tcheck != TMoles) {
    if (vcs_doubleEqual(Tcheck, TMoles)) {
      Tcheck = TMoles;
    } else {
      plogf("We have a consistency problem: %21.16g %21.16g\n",
	     Tcheck, TMoles);
      exit(-1);
    }
  }
}

// Fill in an activity coefficients vector for VCS
/*
 *  This routine will calculate the activity coefficients for the
 *  current phase, and fill in the corresponding entries in the
 *  VCS activity coefficients vector.
 *  
 * @param AC  vector of activity coefficients for all of the species
 *            in all of the phases in a VCS problem. Only the
 *            entries for the current phase are filled in.
 */
void vcs_VolPhase::sendToVCSActCoeff(double * const AC) const {
  if (!m_UpToDate_AC) {
    evaluateActCoeff();
  }
  int kglob;
  for (int k = 0; k < NVolSpecies; k++) {
    kglob = IndSpecies[k];
    AC[kglob] = ActCoeff[k];
  }
}

// Fill in the partial molar volume vector for VCS
/*
 *  This routine will calculate the partial molar volumes for the
 *  current phase (if needed), and fill in the corresponding entries in the
 *  VCS partial molar volumes vector.
 *  
 * @param VolPM  vector of partial molar volumes for all of the species
 *            in all of the phases in a VCS problem. Only the
 *            entries for the current phase are filled in.
 */
double vcs_VolPhase::sendToVCSVolPM(double * const VolPM) const {  
  if (!m_UpToDate_VolPM) {
    (void) VolPM_calc();
  }
  int kglob;
  for (int k = 0; k < NVolSpecies; k++) {
    kglob = IndSpecies[k];
    VolPM[kglob] = PartialMolarVol[k];
  }
  return Vol;
}

// Fill in the partial molar volume vector for VCS
/*
 *  This routine will calculate the partial molar volumes for the
 *  current phase (if needed), and fill in the corresponding entries in the
 *  VCS partial molar volumes vector.
 *  
 * @param VolPM  vector of partial molar volumes for all of the species
 *            in all of the phases in a VCS problem. Only the
 *            entries for the current phase are filled in.
 */
void vcs_VolPhase::sendToVCSGStar(double * const gstar){  
  if (!m_UpToDate_GStar) {
    GStar_calc(Temp, Pres);
  }
  int kglob;
  for (int k = 0; k < NVolSpecies; k++) {
    kglob = IndSpecies[k];
    gstar[kglob] = StarChemicalPotential[k];
  }
}



void vcs_VolPhase::setElectricPotential(double phi) {
  m_phi = phi;
  if (UseCanteraCalls) {
    TP_ptr->setElectricPotential(m_phi);
  }
  // We have changed the state variable. Set uptodate flags to false
  m_UpToDate_AC = false;
  m_UpToDate_VolStar = false;
  m_UpToDate_VolPM = false;
  m_UpToDate_GStar = false;
}

double vcs_VolPhase::electricPotential() const {
  return m_phi;
}

// Sets the temperature and pressure in this object and
//  underlying objects
/*
 *  Sets the temperature and pressure in this object and
 *  underlying objects. The underlying objects refers to the
 *  Cantera's ThermoPhase object for this phase.
 *
 *  @param temperature_Kelvin    (Kelvin)
 *  @param pressure_PA  Pressure (MKS units - Pascal)
 */
void vcs_VolPhase::setState_TP(double temp, double pres)
{
  if (Temp == temp) {
    if (Pres == pres) {
      return;
    }
  }
  if (UseCanteraCalls) {
    TP_ptr->setElectricPotential(m_phi);
    TP_ptr->setState_TP(temp, pres);
  }
  Temp = temp;
  Pres = pres;
  m_UpToDate_AC      = false;
  m_UpToDate_VolStar = false;
  m_UpToDate_VolPM   = false;
  m_UpToDate_GStar   = false;
}


// Molar volume calculation for standard states
/*
 * Calculate the molar volume for the standard states
 * The results are held internally within the object.
 *
 * @param TKelvin Current temperature
 * @param pres    Current pressure
 */
void vcs_VolPhase::VolStar_calc(double tkelvin, double pres) {
  setState_TP(tkelvin, pres);
  if (!m_UpToDate_VolStar) {     
    if (UseCanteraCalls) {
      TP_ptr->getStandardVolumes(VCS_DATA_PTR(StarMolarVol));
    } else {
      for (int k = 0; k < NVolSpecies; k++) {
	int kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarMolarVol[k] =
	  (sTherm->VolStar_calc(kglob, tkelvin, pres));
      }
    }
    m_UpToDate_VolStar = true;
  }
}

// Molar volume calculation for standard state of one species
/*
 * Calculate the molar volume for the standard states
 * The results are held internally within the object.
 * Return the molar volume for one species
 *
 * @param kspec Species number (within the phase)
 * @param TKelvin Current temperature
 * @param pres    Current pressure
 *
 * @return molar volume of the kspec species's standard
 *         state
 */
double vcs_VolPhase::VolStar_calc_one(int kspec, double tkelvin, double pres) 
{
  VolStar_calc(tkelvin, pres);
  return StarMolarVol[kspec];
}

/******************************************************************************
 *
 * VolPM_calc
 */
double vcs_VolPhase::VolPM_calc() const {
  int k, kglob;
  if (!m_UpToDate_VolPM) {
    if (UseCanteraCalls) {
      TP_ptr->getPartialMolarVolumes(VCS_DATA_PTR(PartialMolarVol));
    } else {
      for (k = 0; k < NVolSpecies; k++) {
	kglob = IndSpecies[k];
	vcs_SpeciesProperties *sProp = ListSpeciesPtr[k];
	VCS_SPECIES_THERMO *sTherm = sProp->SpeciesThermo;
	StarMolarVol[k] =
	  (sTherm->VolStar_calc(kglob, Temp, Pres));
      }
      for (k = 0; k < NVolSpecies; k++) {
	PartialMolarVol[k] = StarMolarVol[k];
      }
    }

    Vol = 0.0;
    for (k = 0; k < NVolSpecies; k++) {
      Vol += PartialMolarVol[k] * Xmol[k];
    }
    Vol *= TMoles;
  }
  m_UpToDate_VolPM = true;
  return Vol;
}

/*
 * updateLnActCoeffJac():
 *
 */
void vcs_VolPhase::updateLnActCoeffJac(const double * const moleNumbersVCS) {
  int k, j;
  double deltaMoles_j = 0.0;
  /*
   * Make sure the base state of this object is fully up to date.
   * with the current values of the mole numbers.
   * -> This sets TMoles and Xmol[]
   */
  setMolesFromVCS(moleNumbersVCS);

  /*
   * Evaluate the current base activity coefficients.
   */  
  evaluateActCoeff();

  // Make copies of ActCoeff and Xmol for use in taking differences
  std::vector<double> ActCoeff_Base(ActCoeff);
  std::vector<double> Xmol_Base(Xmol);
  double TMoles_base = TMoles;

  /*
   *  Loop over the columns species to be deltad
   */
  for (j = 0; j < NVolSpecies; j++) {
    /*
     * Calculate a value for the delta moles of species j
     * -> NOte Xmol[] and Tmoles are always positive or zero
     *    quantities.
     */
    double moles_j_base = TMoles * Xmol_Base[j];
    deltaMoles_j = 1.0E-7 * moles_j_base + 1.0E-20 * TMoles + 1.0E-150;
    /*
     * Now, update the total moles in the phase and all of the
     * mole fractions based on this.
     */
    TMoles = TMoles_base + deltaMoles_j;      
    for (k = 0; k < NVolSpecies; k++) {
      Xmol[k] = Xmol_Base[k] * TMoles_base / TMoles;
    }
    Xmol[j] = (moles_j_base + deltaMoles_j) / TMoles;
 
    /*
     * Go get new values for the activity coefficients.
     * -> Note this calls setState_PX();
     */
    _updateMoleFractionDependencies();
    evaluateActCoeff();
    /*
     * Calculate the column of the matrix
     */
    double * const lnActCoeffCol = dLnActCoeffdMolNumber[j];
    for (k = 0; k < NVolSpecies; k++) {
      lnActCoeffCol[k] = (ActCoeff[k] - ActCoeff_Base[k]) /
	((ActCoeff[k] + ActCoeff_Base[k]) * 0.5 * deltaMoles_j);
    }
    /*
     * Revert to the base case Xmol, TMoles
     */
    TMoles = TMoles_base;
    vcs_vdcopy(Xmol, Xmol_Base, NVolSpecies);
  }
  /*
   * Go get base values for the activity coefficients.
   * -> Note this calls setState_TPX() again;
   * -> Just wanted to make sure that cantera is in sync
   *    with VolPhase after this call.
   */
  setMoleFractions(VCS_DATA_PTR(Xmol_Base));
  _updateMoleFractionDependencies();
  evaluateActCoeff();
}

// Downloads the ln ActCoeff jacobian into the VCS version of the
// ln ActCoeff jacobian.
/*
 *
 *   This is essentially a scatter operation.
 *
 *   The Jacobians are actually d( lnActCoeff) / d (MolNumber);
 *   dLnActCoeffdMolNumber[j][k]
 * 
 *      j = id of the species mole number
 *      k = id of the species activity coefficient
 */
void vcs_VolPhase::sendToVCSLnActCoeffJac(double * const * const LnACJac_VCS) const {
  int j, k, jglob, kglob;
  for (j = 0; j < NVolSpecies; j++) {
    jglob = IndSpecies[j];
    double * const lnACJacVCS_col = LnACJac_VCS[jglob];
    const double * const lnACJac_col = dLnActCoeffdMolNumber[j];
    for (k = 0; k < NVolSpecies; k++) {
      kglob = IndSpecies[k];
      lnACJacVCS_col[kglob] = lnACJac_col[k];
    }
  }
}

// Set the pointer for Cantera's ThermoPhase parameter
/*
 *  When we first initialize the ThermoPhase object, we read the
 *  state of the ThermoPhase into vcs_VolPhase object.
 *
 * @param tp_ptr Pointer to the ThermoPhase object corresponding
 *               to this phase.
 */
void vcs_VolPhase::setPtrThermoPhase(Cantera::ThermoPhase *tp_ptr) {
  TP_ptr = tp_ptr;
  if (TP_ptr) {
    UseCanteraCalls = true;
    Temp = TP_ptr->temperature();
    Pres = TP_ptr->pressure();
    setState_TP(Temp, Pres);
    m_VCS_UnitsFormat = VCS_UNITS_MKS;
    m_phi = TP_ptr->electricPotential();
    int nsp = TP_ptr->nSpecies();
    if (nsp !=  NVolSpecies) {
      if (NVolSpecies != 0) {
	plogf("Warning Nsp != NVolSpeces: %d %d \n", nsp, NVolSpecies);
      }
      resize(VP_ID, nsp, PhaseName.c_str());
    }
    TP_ptr->getMoleFractions(VCS_DATA_PTR(Xmol));
    _updateMoleFractionDependencies();
  } else {
    UseCanteraCalls = false;
  }
}

// Return a const ThermoPhase pointer corresponding to this phase
/*
 *  @return pointer to the ThermoPhase.
 */
const Cantera::ThermoPhase *vcs_VolPhase::ptrThermoPhase() const {
  return TP_ptr;
}

double vcs_VolPhase::TotalMoles() const {
  return TMoles;
}

double vcs_VolPhase::molefraction(int k) const {
  return Xmol[k];
}

void vcs_VolPhase::setTotalMoles(double tmols)  {
  TMoles = tmols;
}

// Return a string representing the equation of state
/* 
 * The string is no more than 16 characters. 
 *  @param EOSType : integer value of the equation of state
 *
 * @return returns a string representing the EOS
 */
std::string string16_EOSType(int EOSType) {
  char st[32];
  st[16] = '\0';
  switch (EOSType) {
  case VCS_EOS_CONSTANT:
    sprintf(st,"Constant        ");
    break;
  case VCS_EOS_IDEAL_GAS:
    sprintf(st,"Ideal Gas       ");
    break;
  case  VCS_EOS_STOICH_SUB:
    sprintf(st,"Stoich Sub      ");
    break;
  case VCS_EOS_IDEAL_SOLN:
    sprintf(st,"Ideal Soln      ");
    break;
  case VCS_EOS_DEBEYE_HUCKEL:
    sprintf(st,"Debeye Huckel   ");
    break;
  case VCS_EOS_REDLICK_KWONG:
    sprintf(st,"Redlick_Kwong   ");
    break;
  case VCS_EOS_REGULAR_SOLN:
    sprintf(st,"Regular Soln    ");
    break;
  default:
    sprintf(st,"UnkType: %-7d", EOSType);
    break;
  }
  st[16] = '\0';
  std::string sss=st;
  return sss;
}

}

