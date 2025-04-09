#include <dlib/svm_threaded.h>
#include "classification/classifier_data.h"
#include "classification/intent_trainer.h"
#include "types/definitions.h"

using namespace dlib;

int main() {
    intent_trainer action_trainer("intents/action");
    intent_trainer object_trainer("intents/object");

    action_trainer.build_vocabulary();
    object_trainer.build_vocabulary();

    action_trainer.train();
    object_trainer.train();

    action_trainer.save("models/action_classifier");
    object_trainer.save("models/object_classifier");
    /*train_classifier("intents/action", "action_classifier.dat");
    train_classifier("intents/object", "object_classifier.dat");*/
    //std::vector<std::string> texts;
    //std::vector<unsigned long> labels;

    //std::vector<text_label> samples_with_labels = load_label_mapping("intents");

    //if (samples_with_labels.empty()) {
    //    std::cerr << "[Error] No text or labels found";
    //    return -1;
    //}

    //for (const auto& sample : samples_with_labels) {
    //    texts.push_back(sample.first);
    //    labels.push_back(sample.second);
    //}

    //std::set<std::string> vocabulary = build_vocabulary(texts);
    //save_vocabulary(vocabulary);
    ///*std::vector<sample_type> samples = get_samples(texts);*/
    //std::vector<sample_type> samples = preprocess_text(texts, vocabulary);

    //if (samples.empty()) {
    //    std::cerr << "[Error] No samples extracted.\n";
    //    return -1;
    //}


    //svm_c_trainer<linear_kernel<sample_type>> linear_trainer;
    //linear_trainer.set_c(10);

    //ovo_trainer trainer;
    //trainer.set_trainer(linear_trainer);

    ////auto df = trainer.train(samples, labels);

    //df_type df_serializable = trainer.train(samples, labels);
    //

    //serialize("mlp_command_classifier.dat") << df_serializable;
    //std::cout << "[Info] Model trained and saved to mlp_command_classifier.dat\n";
    //
    return 0;
}