#pragma once
#include <dlib/svm_threaded.h>
#include <dlib/mlp.h>

using namespace dlib;

typedef unsigned long ulong;
using sample_type = matrix<double, 0, 1>;
using label_type = ulong;
using text_label = std::pair<std::string, label_type>;
using trainer_type = svm_c_trainer<linear_kernel<sample_type>>;
using ovo_trainer = one_vs_one_trainer<any_trainer<sample_type>, label_type>;
using df_type = one_vs_one_decision_function<ovo_trainer, decision_function<linear_kernel<sample_type>>>;