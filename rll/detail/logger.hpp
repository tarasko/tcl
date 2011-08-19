#ifndef __RLDREAM_LOGGER_H__
#define __RLDREAM_LOGGER_H__

#include "../types.hpp"
#include "../value_function.hpp"

#include "fann/doublefann.h"

#include <stdio.h>

namespace tcl { namespace rll {

/** @brief Helps to log library events to rl_log.txt */
class CLogger {
public:
  CLogger(void) : m_pFile(NULL) {}
  ~CLogger() {close();}

  /** @brief Enable or disable logging */
  void                  Enable(bool i_enable);
  /** @name Set of logging functions */
  //@{
  void                  Print(const char* i_who, const char* i_desc);
  void                  Print(const CVectorDbl& i_vector);
  void                  Print(const char* i_who, const char* i_desc,
                              const CVectorDbl& i_vector);
  void                  Print(const char* i_who, const char* i_desc,
                              CValueFunction::CUpdateList& i_list);
  void                  Print(fann_train_data i_td);
  //@}
protected:
  /** @brief Close and free file stream */
  void                  close();
  /** @brief Output file stream */
  FILE*                 m_pFile;
};

extern CLogger g_log;

}}

#endif //__RLDREAM_LOGGER_H__
