/*
* Author   : Saurabh Shrivastava
* Email    : saurabh.shrivastava54@gmail.com
* Link     : https://github.com/saurabhshri
*/

#include "params.h"

// Default paths.
namespace {
    constexpr auto defaultModelPath = "model/";
    constexpr auto defaultLmPath = "tempFiles/lm/complete.lm";
    constexpr auto defaultDictPath = "tempFiles/dict/complete.dict";
    constexpr auto defaultFsgPath = "tempFiles/fsg/";
    constexpr auto defaultPhoneticLmPath = "model/en-us-phone.lm.bin";
}

Params::Params() noexcept
    : localTime(32, '\0'),

    modelPath(defaultModelPath),
    lmPath(defaultLmPath),
    dictPath(defaultDictPath),
    fsgPath(defaultFsgPath),
    phoneticlmPath(defaultPhoneticLmPath),

    searchWindow(3),
    audioWindow(0),
    sampleWindow(0),

    chosenAlignerType(asrAligner),
    grammarType(complete_grammar),
    outputFormat(xml),
    printOption(printBothWithDistinctColors),
    useFSG(),
    transcribe(),
    useBatchMode(),
    useExperimentalParams(),
    searchPhonemes(),
    displayRecognised(true),
    readStream(),
    quickDict(),
    quickLM(),
    audioIsRaw() {
    // Using date and time for log filename.
    const auto now = std::time(nullptr);
    localTime.erase(std::strftime(&localTime.front(), localTime.size(), "%d-%m-%Y-%H-%M-%S", std::localtime(&now)));

    logPath = "tempFiles/" + localTime + ".log";
    alignerLogPath = "tempFiles/aligner-" + localTime + ".log";
    phonemeLogPath = "tempFiles/phoneme-" + localTime + ".log";
}

void Params::inputParams(int argc, char *argv[]) {
    for (int i = 1; i<argc; i++)         //parsing arguments
    {
        std::string subParam, paramPrefix(argv[i]);

        if (i + 1 < argc)
            subParam = argv[i + 1];

        else
            subParam = "";

        if (paramPrefix == "-wav") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-wav requires a path to valid wave!";
            }

            audioFileName = subParam;
            i++;
        }
        else if (paramPrefix == "-raw") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-raw requires a path to valid raw wave!";
            }

            audioFileName = subParam;
            audioIsRaw = true;
            i++;
        }
        else if (paramPrefix == "--raw-stream") {
            audioIsRaw = true;
            readStream = true;
        }
        else if (paramPrefix == "-srt") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-srt requires a path to valid SubRip subtitle file!";
            }

            usingTranscript = false;
            subtitleFileName = subParam;
            i++;
        }
        else if (paramPrefix == "-txt") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-txt requires a path to valid transcript file!";
            }

            usingTranscript = true;
            transcriptFileName = subParam;
            i++;
        }
        else if (paramPrefix == "-" || paramPrefix == "-stdin") {
            readStream = true;
        }

        else if (paramPrefix == "-out") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-out requires a valid output filename!";
            }

            outputFileName = subParam;
            i++;
        }

        else if (paramPrefix == "-oFormat") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-oFormat requires a valid output format";
            }

            if (subParam == "srt")
                outputFormat = srt;

            else if (subParam == "xml")
                outputFormat = xml;

            else if (subParam == "json")
                outputFormat = json;

            else if (subParam == "karaoke")
                outputFormat = karaoke;

            else if (subParam == "stdout")
                outputFormat = console;

            else {
                FATAL(EXIT_INVALID_PARAMETERS) << "-oFormat requires a valid output format!";
            }

            i++;
        }

        else if (paramPrefix == "-model") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-model requires a path to valid acoustic model!";
            }

            modelPath = subParam;
            i++;
        }

        else if (paramPrefix == "-lm") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-lm requires a path to language model!";
            }

            lmPath = subParam;
            i++;
        }


        else if (paramPrefix == "-dict") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-dict requires a path to a valid dictionary file!";
            }

            dictPath = subParam;
            i++;
        }


        else if (paramPrefix == "-fsg") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-fsg requires a path to a valid directory containing FSG files!";
            }

            fsgPath = subParam;
            i++;
        }


        else if (paramPrefix == "-log") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-log requires a path to a valid output log file!";
            }

            logPath = subParam;
            i++;
        }

        else if (paramPrefix == "-phoneLM") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-phoneLM requires a path to a valid phonetic language model!";

            }

            phoneticlmPath = subParam;
            i++;
        }

        else if (paramPrefix == "-alignerLog") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-alignerLog requires a path to a valid output log file!";

            }

            alignerLogPath = subParam;
            i++;
        }

        else if (paramPrefix == "-phoneLog") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-phoneLog requires a path to a valid output log file!";

            }

            phonemeLogPath = subParam;
            i++;
        }

        else if (paramPrefix == "--enable-phonemes") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--enable-phonemes requires a valid response!";

            }

            if (subParam == "yes")
                searchPhonemes = true;

            i++;
        }

        else if (paramPrefix == "--generate-grammar") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--generate-grammar requires a valid response!";

            }

            if (subParam == "yes")
                grammarType = complete_grammar;

            else if (subParam == "no")
                grammarType = no_grammar;

            else if (subParam == "onlyCorpus")
                grammarType = corpus;

            else if (subParam == "onlyDict")
                grammarType = dict;

            else if (subParam == "onlyFSG")
                grammarType = fsg;

            else if (subParam == "onlyLM")
                grammarType = lm;

            else if (subParam == "onlyVocab")
                grammarType = vocab;

            else {
                FATAL(EXIT_INVALID_PARAMETERS) << "--generate-grammar requires a valid response!";
            }

            i++;
        }

        else if (paramPrefix == "--quick-dict") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--quick-dict requires a valid response!";
            }

            if (subParam == "yes")
                quickDict = true;

            i++;
        }

        else if (paramPrefix == "--quick-lm") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--quick-lm requires a valid response!";
            }

            if (subParam == "yes")
                quickLM = true;

            i++;
        }

        else if (paramPrefix == "--print-aligned") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--print-aligned requires a valid response!";
            }

            if (subParam == "yes")
                printOption = printOnlyRecognised;

            else if (subParam == "no")
                printOption = printBothWihoutColors;

            else if (subParam == "withColors")
                printOption = printBothWithDistinctColors;

            else if (subParam == "karaoke")
                printOption = printAsKaraoke;

            else if (subParam == "karaokeWithColors")
                printOption = printAsKaraoke;

            else {
                FATAL(EXIT_INVALID_PARAMETERS) << "--print-aligned requires a valid response!";
            }

            i++;
        }

        else if (paramPrefix == "--use-fsg") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--use-fsg requires a valid response!";
            }

            if (subParam == "yes")
                useFSG = true;

            i++;
        }

        else if (paramPrefix == "-transcribe") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-transcribe requires a valid response!";
            }

            if (subParam == "yes")
                transcribe = true;

            i++;
        }


        else if (paramPrefix == "-verbosity") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-verbose requires a valid response!";
            }

            if (subParam == "verbose")
                getLogger().setMinimumOutputLevel(Logger::Level::verbose);
            else if (subParam == "debug")
                getLogger().setMinimumOutputLevel(Logger::Level::debug);
            else if (subParam == "info")
                getLogger().setMinimumOutputLevel(Logger::Level::info);
            else if (subParam == "warning")
                getLogger().setMinimumOutputLevel(Logger::Level::warning);
            else if (subParam == "error")
                getLogger().setMinimumOutputLevel(Logger::Level::error);
            else if (subParam == "nolog")
                getLogger().setMinimumOutputLevel(Logger::Level::nolog);

            i++;
        }

        else if (paramPrefix == "--display-recognised") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "--display-recognised requires a valid response!";
            }

            if (subParam == "no")
                displayRecognised = false;

            i++;
        }

        else if (paramPrefix == "-searchWindow") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-searchWindow requires an integer value to determine the search scope!";
            }

            searchWindow = std::strtoul(subParam.c_str(), nullptr, 10);

            if (errno) {
                FATAL(EXIT_FAILURE) << "Invalid value passed to -searchWindow : " << strerror(errno);
            }

            i++;
        }

        else if (paramPrefix == "-sampleWindow") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-sampleWindow requires a valid integer value to determine the recognition scope!";
            }

            sampleWindow = std::strtoul(subParam.c_str(), nullptr, 10);

            if (errno) {
                FATAL(EXIT_FAILURE) << "Invalid value passed to -sampleWindow : " << strerror(errno);
            }

            i++;
        }

        else if (paramPrefix == "-audioWindow") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-audioWindow requires a valid integer value in milliseconds to determine the recognition scope!";
            }

            audioWindow = std::strtoul(subParam.c_str(), nullptr, 10);

            if (errno) {
                FATAL(EXIT_FAILURE) << "Invalid value passed to -audioWindow : " << strerror(errno);
            }

            i++;
        }

        else if (paramPrefix == "-useBatchMode") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-useBatchMode requires a valid response!";
            }

            if (subParam == "yes")
                useBatchMode = true;

            i++;
        }

        else if (paramPrefix == "-experiment") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-experiment requires a valid response!";
            }

            if (subParam == "yes")
                useExperimentalParams = true;

            i++;
        }

        else if (paramPrefix == "-approx") {
            if (i + 1 > argc) {
                FATAL(EXIT_INCOMPLETE_PARAMETERS) << "-approx requires a valid response!";
            }

            if (subParam == "yes")
                chosenAlignerType = approxAligner;

            i++;
        }

        else {
            FATAL(EXIT_INVALID_PARAMETERS) << "Parameter '" << paramPrefix << "' is not recognised!";
        }

    }

    validateParams();

}

void Params::validateParams() {
    if (audioFileName.empty() && !readStream)
        FATAL(EXIT_INVALID_PARAMETERS) << "Audio file name is empty!";

    if (subtitleFileName.empty() && !usingTranscript)
        FATAL(EXIT_INVALID_PARAMETERS) << "Subtitle file name is empty!";

    if (transcriptFileName.empty() && usingTranscript)
        FATAL(EXIT_INVALID_FILE) << "Transcript file name is empty!";

    if (usingTranscript && chosenAlignerType == approxAligner)
        FATAL(EXIT_INVALID_PARAMETERS) << "Approx alligner doesn't work with text files";

    if (modelPath.empty())
        DEBUG << "Using default Model Path.";

    if (dictPath.empty())
        DEBUG << "Using default Dictionary Path.";

    if (lmPath.empty())
        DEBUG << "Using default LM Path.";

    if (logPath.empty())
        DEBUG << "Using default Log Path.";

    if (fsgPath.empty())
        DEBUG << "Using default FSG Path.";

    if (phoneticlmPath.empty())
        DEBUG << "Using default Phonetic LM Path.";

    if (phonemeLogPath.empty())
        DEBUG << "Using default Phoneme Log Path.";

    if (alignerLogPath.empty())
        DEBUG << "Using default Aligner Log Path.";

    if (readStream) {
        audioFileName = "stdin";
    }

    if (outputFileName.empty()) {
        outputFileName = extractFileName(audioFileName);

        switch (outputFormat)  //decide on basis of set output format
        {
        case srt:       outputFileName += ".srt";
            break;

        case xml:       outputFileName += ".xml";
            break;

        case json:      outputFileName += ".json";
            break;

        case karaoke:   outputFileName += ".srt";
            break;

        default:        FATAL(EXIT_UNKNOWN) << "An error occurred while choosing output format!";
            exit(2);
        }
    }

    if (grammarType == complete_grammar && quickDict)
        grammarType = quick_dict;

    if (grammarType == complete_grammar && quickLM)
        grammarType = quick_lm;

    if (useFSG && transcribe) {
        FATAL(EXIT_INCOMPATIBLE_PARAMETERS) << "FSG and Transcribing are not compatible!";
    }

    if (searchPhonemes && transcribe) {
        FATAL(EXIT_INCOMPATIBLE_PARAMETERS) << "Sorry, currently phoneme transcribing is not supported!";
    }

    printParams();
}

void Params::printParams() const noexcept {
    VERBOSE << "audioFileName       : " << audioFileName;
    if (!usingTranscript)
        VERBOSE << "subtitleFileName    : " << subtitleFileName;
    else
        VERBOSE << "transcriptFileName	: " << transcriptFileName;
    VERBOSE << "outputFileName      : " << outputFileName;
    VERBOSE << "modelPath           : " << modelPath;
    VERBOSE << "lmPath              : " << lmPath;
    VERBOSE << "dictPath            : " << dictPath;
    VERBOSE << "fsgPath             : " << fsgPath;
    VERBOSE << "phoneticlmPath      : " << phoneticlmPath;
    VERBOSE << "logPath             : " << logPath;
    VERBOSE << "phonemeLogPath      : " << phonemeLogPath;
    VERBOSE << "alignerLogPath      : " << alignerLogPath;
    VERBOSE << "sampleWindow        : " << sampleWindow;
    VERBOSE << "audioWindow         : " << audioWindow;
    VERBOSE << "searchWindow        : " << searchWindow;
    VERBOSE << "chosenAlignerType   : " << chosenAlignerType;
    VERBOSE << "grammarType         : " << grammarType;
    VERBOSE << "outputFormat        : " << outputFormat;
    VERBOSE << "printOption         : " << printOption;
    VERBOSE << "verbosity           : " << verbosity;
    VERBOSE << "useFSG              : " << useFSG;
    VERBOSE << "transcribe          : " << transcribe;
    VERBOSE << "useBatchMode        : " << useBatchMode;
    VERBOSE << "ExperimentalParams  : " << useExperimentalParams;
    VERBOSE << "searchPhonemes      : " << searchPhonemes;
    VERBOSE << "displayRecognised   : " << displayRecognised;
    VERBOSE << "readStream          : " << readStream;
    VERBOSE << "quickDict           : " << quickDict;
    VERBOSE << "quickLM             : " << quickLM;
    VERBOSE << "\n\n=====================================================\n";
}
