#include "logger.hpp"

namespace tcl { namespace rll {

CLogger g_log;

using boost::any_cast;

void CLogger::close() {
  if (NULL != m_pFile) {
    fclose(m_pFile);
    m_pFile = NULL;
  }
}

void CLogger::Enable(bool i_enable) {
  close();
  if (i_enable) {
    m_pFile = fopen("rl_log.txt", "w+t");
  }
}

void CLogger::Print(const char* i_who,
                    const char* i_desc, 
                    const CVectorDbl& i_vector) {
  Print(i_who, i_desc);
  Print(i_vector);
}

void CLogger::Print(const CVectorDbl& i_vector) {
  if (!m_pFile) {
    return;
  }

  for (CVectorDbl::const_iterator i = i_vector.begin(); i != i_vector.end(); ++i) {
    if (typeid(int) == i->type()) {
      fprintf(m_pFile, "%i ", any_cast<int>(*i));
    } else if (typeid(double) == i->type()) {
      fprintf(m_pFile, "%8f ", any_cast<double>(*i));
    } else {
      fprintf(m_pFile, "\nWARNING: Unexpected type inside of any\n");
      return;
    }
  }
  fprintf(m_pFile, "\n");
}

void CLogger::Print(const char* i_who, const char* i_desc) {
  if (!m_pFile) {
    return;
  }
  fprintf(m_pFile, "%s: %s\n", i_who, i_desc);
}

void CLogger::Print(fann_train_data i_td) {
  if (!m_pFile) {
    return;
  }
  fprintf(m_pFile, "Input dimension: %u. Output dimension: %u. Examples: %u\n", 
    i_td.num_input, i_td.num_output, i_td.num_data);
  for (unsigned int ex=0; ex<i_td.num_data; ++ex) {
    fprintf(m_pFile, "Example: %u\n", ex);
    fprintf(m_pFile, "(");
    double* cur_input = i_td.input[ex];
    double* cur_output = i_td.output[ex];
    for (unsigned int in_n=0; in_n<i_td.num_input; ++in_n) {
      fprintf(m_pFile, "%f, ", cur_input[in_n]);
    }
    fprintf(m_pFile, ") - (");
    for (unsigned int out_n=0; out_n<i_td.num_output; ++out_n) {
      fprintf(m_pFile, "%f, ", cur_output[out_n]);
    }
    fprintf(m_pFile, ")\n");
  }
  fprintf(m_pFile, "\n");
}

void CLogger::Print(const char* i_who, const char* i_desc,
                    CValueFunction::CUpdateList& i_list) {
  typedef CValueFunction::CUpdateList CUpdateList;

  if (!m_pFile) {
    return;
  }
  fprintf(m_pFile, "[%s] %s:\n", i_who, i_desc);
  for (CUpdateList::iterator i=i_list.begin(); i!=i_list.end(); ++i) {
    fprintf(m_pFile, "%f: ", i->second); 
    Print(*i->first);
  }
}

}}

