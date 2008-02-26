#ifndef __OCTAVEINTERFACE__H_
#define __OCTAVEINTERFACE__H_

#include "lib/config.h"

#if defined(HAVE_OCTAVE) && !defined(HAVE_SWIG)            
#include "interface/SGInterface.h"

#include <octave/config.h>

#include <octave/defun-dld.h>
#include <octave/error.h>
#include <octave/oct-obj.h>
#include <octave/pager.h>
#include <octave/symtab.h>
#include <octave/variables.h>

class COctaveInterface : public CSGInterface
{
	public:
		COctaveInterface(octave_value_list prhs, INT nlhs);
		~COctaveInterface();

		/** get functions - to pass data from the target interface to shogun */
		virtual void parse_args(INT num_args, INT num_default_args);

		/// get type of current argument (does not increment argument counter)
		virtual IFType get_argument_type();

		virtual INT get_int();
		virtual DREAL get_real();
		virtual bool get_bool();

		virtual CHAR* get_string(INT& len);

		virtual void get_byte_vector(BYTE** vec, INT* len);
		virtual void get_int_vector(INT** vec, INT* len);
		virtual void get_shortreal_vector(SHORTREAL** vec, INT* len);
		virtual void get_real_vector(DREAL** vec, INT* len);

		virtual void get_byte_matrix(BYTE** matrix, INT* num_feat, INT* num_vec);
		virtual void get_int_matrix(INT** matrix, INT* num_feat, INT* num_vec);
		virtual void get_shortreal_matrix(SHORTREAL** matrix, INT* num_feat, INT* num_vec);
		virtual void get_real_matrix(DREAL** matrix, INT* num_feat, INT* num_vec);

		virtual void get_byte_sparsematrix(TSparse<BYTE>** matrix, INT* num_feat, INT* num_vec);
		virtual void get_int_sparsematrix(TSparse<INT>** matrix, INT* num_feat, INT* num_vec);
		virtual void get_shortreal_sparsematrix(TSparse<SHORTREAL>** matrix, INT* num_feat, INT* num_vec);
		virtual void get_real_sparsematrix(TSparse<DREAL>** matrix, INT* num_feat, INT* num_vec);

		virtual void get_string_list(T_STRING<CHAR>** strings, INT* num_str);

		/** set functions - to pass data from shogun to the target interface */
		virtual void create_return_values(INT num_val);
		virtual void set_byte_vector(BYTE* vec, INT len);
		virtual void set_int_vector(INT* vec, INT len);
		virtual void set_shortreal_vector(SHORTREAL* vec, INT len);
		virtual void set_real_vector(DREAL* vec, INT len);

		virtual void set_byte_matrix(BYTE* matrix, INT num_feat, INT num_vec);
		virtual void set_int_matrix(INT* matrix, INT num_feat, INT num_vec);
		virtual void set_shortreal_matrix(SHORTREAL* matrix, INT num_feat, INT num_vec);
		virtual void set_real_matrix(DREAL* matrix, INT num_feat, INT num_vec);

		virtual void set_byte_sparsematrix(TSparse<BYTE>* matrix, INT num_feat, INT num_vec);
		virtual void set_int_sparsematrix(TSparse<INT>* matrix, INT num_feat, INT num_vec);
		virtual void set_shortreal_sparsematrix(TSparse<SHORTREAL>* matrix, INT num_feat, INT num_vec);
		virtual void set_real_sparsematrix(TSparse<DREAL>* matrix, INT num_feat, INT num_vec);

		virtual void set_string_list(T_STRING<CHAR>* strings, INT num_str);

		virtual void submit_return_values();

		inline octave_value_list get_return_values()
		{
			return m_lhs;
		}

	private:
		const octave_value get_current_arg()
		{
			ASSERT(arg_counter>=0 && arg_counter<m_nrhs+1); // +1 for action
			return m_rhs(arg_counter);
		}

	private:
		octave_value_list m_lhs;
		octave_value_list m_rhs;
};
#endif // HAVE_OCTAVE && HAVE_SWIG
#endif // __OCTAVEINTERFACE__H_
