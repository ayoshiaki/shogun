/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Soeren Sonnenburg, Chiyuan Zhang, Sergey Lisitsyn, Thoralf Klein, 
 *          Viktor Gal, Weijie Lin, Evan Shelhamer, Sanuj Sharma
 */

#include <shogun/classifier/svm/OnlineLibLinear.h>
#include <shogun/features/streaming/StreamingDenseFeatures.h>
#include <shogun/features/streaming/StreamingSparseFeatures.h>
#include <shogun/mathematics/Math.h>
#include <shogun/mathematics/linalg/LinalgNamespace.h>
#include <shogun/lib/Time.h>

using namespace shogun;

COnlineLibLinear::COnlineLibLinear()
	: COnlineLinearMachine()
{
		init();
}

COnlineLibLinear::COnlineLibLinear(float64_t C_reg)
	: COnlineLinearMachine()
{
		init();
		C1=C_reg;
		C2=C_reg;
		use_bias=true;
}

COnlineLibLinear::COnlineLibLinear(
		float64_t C_reg, CStreamingDotFeatures* traindat)
	: COnlineLinearMachine()
{
		init();
		C1=C_reg;
		C2=C_reg;
		use_bias=true;

		set_features(traindat);
}

COnlineLibLinear::COnlineLibLinear(COnlineLibLinear *mch)
	: COnlineLinearMachine()
{
	init();
	C1 = mch->C1;
	C2 = mch->C2;
	use_bias = mch->use_bias;

	set_features(mch->features);
	m_w = mch->m_w.clone();
	bias = mch->bias;
}


void COnlineLibLinear::init()
{
	C1=1;
	C2=1;
	Cp=1;
	Cn=1;
	use_bias=false;

	SG_ADD(&C1, "C1", "C Cost constant 1.", ParameterProperties::HYPER);
	SG_ADD(&C2, "C2", "C Cost constant 2.", ParameterProperties::HYPER);
	SG_ADD(
	    &use_bias, "use_bias", "Indicates if bias is used.", ParameterProperties::SETTING);

	PG = 0;
	PGmax_old = CMath::INFTY;
	PGmin_old = -CMath::INFTY;
	PGmax_new = -CMath::INFTY;
	PGmin_new = CMath::INFTY;

	diag[0]=0;diag[1]=0;diag[2]=0;
	upper_bound[0]=Cn;upper_bound[1]=0;upper_bound[2]=Cp;

	v = 0;
	nSV = 0;

	// TODO: "local" variables only used in one method
	C = 0;
	d = 0;
	G = 0;
	QD = 0;
	alpha_current = 0;
}

COnlineLibLinear::~COnlineLibLinear()
{
}

void COnlineLibLinear::start_train()
{
	Cp = C1;
	Cn = C2;
	bias = false;

	PGmax_old = CMath::INFTY;
	PGmin_old = -CMath::INFTY;
	PGmax_new = -CMath::INFTY;
	PGmin_new = CMath::INFTY;

	diag[0]=0;diag[1]=0;diag[2]=0;
	upper_bound[0]=Cn;upper_bound[1]=0;upper_bound[2]=Cp;

	v = 0;
	nSV = 0;
}

void COnlineLibLinear::stop_train()
{
	float64_t gap = PGmax_new - PGmin_new;

	io::progress_done();
	io::info("Optimization finished.");

	// calculate objective value
	v = linalg::dot(m_w, m_w);
	v += bias*bias;

	io::info("Objective value = {}", v/2);
	io::info("nSV = {}", nSV);
	io::info("gap = {:g}", gap);
}

void COnlineLibLinear::train_one(SGVector<float32_t> ex, float64_t label)
{
	alpha_current = 0;
	int32_t y_current = 0;
	if (label > 0)
		y_current = +1;
	else
		y_current = -1;

	QD = diag[y_current + 1];
	// Dot product of vector with itself
	QD += linalg::dot(ex, ex);

	// Dot product of vector with learned weights
	G = linalg::dot(ex, m_w);

	if (use_bias)
		G += bias;
	G = G*y_current - 1;
	// LINEAR TERM PART?

	C = upper_bound[y_current + 1];
	G += alpha_current*diag[y_current + 1]; // Can be eliminated, since diag = 0 vector

	PG = 0;
	if (alpha_current == 0) // This condition will always be true in the online version
	{
		if (G > PGmax_old)
		{
			return;
		}
		else if (G < 0)
			PG = G;
	}
	else if (alpha_current == C)
	{
		if (G < PGmin_old)
		{
			return;
		}
		else if (G > 0)
			PG = G;
	}
	else
		PG = G;

	PGmax_new = CMath::max(PGmax_new, PG);
	PGmin_new = CMath::min(PGmin_new, PG);

	if (fabs(PG) > 1.0e-12)
	{
		float64_t alpha_old = alpha_current;
		alpha_current = CMath::min(CMath::max(alpha_current - G/QD, 0.0), C);
		d = (alpha_current - alpha_old) * y_current;

		linalg::add(m_w, ex, m_w, 1.0f, (float32_t)d);

		if (use_bias)
			bias += d;
	}

	v += alpha_current*(alpha_current*diag[y_current + 1] - 2);
	if (alpha_current > 0)
		nSV++;
}

void COnlineLibLinear::train_one(SGSparseVector<float32_t> ex, float64_t label)
{
	alpha_current = 0;
	int32_t y_current = 0;
	if (label > 0)
		y_current = +1;
	else
		y_current = -1;

	QD = diag[y_current + 1];
	// Dot product of vector with itself
	QD += SGSparseVector<float32_t>::sparse_dot(ex, ex);

	// Dot product of vector with learned weights
	G = ex.dense_dot(1.0,m_w.vector,m_w.vlen,0.0);

	if (use_bias)
		G += bias;
	G = G*y_current - 1;
	// LINEAR TERM PART?

	C = upper_bound[y_current + 1];
	G += alpha_current*diag[y_current + 1]; // Can be eliminated, since diag = 0 vector

	PG = 0;
	if (alpha_current == 0) // This condition will always be true in the online version
	{
		if (G > PGmax_old)
		{
			return;
		}
		else if (G < 0)
			PG = G;
	}
	else if (alpha_current == C)
	{
		if (G < PGmin_old)
		{
			return;
		}
		else if (G > 0)
			PG = G;
	}
	else
		PG = G;

	PGmax_new = CMath::max(PGmax_new, PG);
	PGmin_new = CMath::min(PGmin_new, PG);

	if (fabs(PG) > 1.0e-12)
	{
		float64_t alpha_old = alpha_current;
		alpha_current = CMath::min(CMath::max(alpha_current - G/QD, 0.0), C);
		d = (alpha_current - alpha_old) * y_current;

		for (int32_t i=0; i < ex.num_feat_entries; i++)
			m_w[ex.features[i].feat_index] += d*ex.features[i].entry;


		if (use_bias)
			bias += d;
	}

	v += alpha_current*(alpha_current*diag[y_current + 1] - 2);
	if (alpha_current > 0)
		nSV++;
}

void COnlineLibLinear::train_example(CStreamingDotFeatures *feature, float64_t label)
{
	feature->expand_if_required(m_w.vector, m_w.vlen);

	if (feature->get_feature_class() == C_STREAMING_DENSE) {
		CStreamingDenseFeatures<float32_t> *feat =
			dynamic_cast<CStreamingDenseFeatures<float32_t> *>(feature);
		if (feat == NULL)
			error("Expected streaming dense feature <float32_t>");

		train_one(feat->get_vector(), label);
	}
	else if (feature->get_feature_class() == C_STREAMING_SPARSE) {
		CStreamingSparseFeatures<float32_t> *feat =
			dynamic_cast<CStreamingSparseFeatures<float32_t> *>(feature);
		if (feat == NULL)
			error("Expected streaming sparse feature <float32_t>");

		train_one(feat->get_vector(), label);
	}
	else {
		not_implemented(SOURCE_LOCATION);
	}
}
