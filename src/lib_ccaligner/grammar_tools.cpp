/*
 * Author   : Saurabh Shrivastava
 * Email    : saurabh.shrivastava54@gmail.com
 * Link     : https://github.com/saurabhshri
*/

#include "grammar_tools.h"

static int systemGetStatus(const char* command) {
    int rv = std::system(command);

#ifndef WIN32
    if (WIFEXITED(rv) == 0) return 0;
    return WEXITSTATUS(rv);
#else
    return rv;
#endif
}

bool generate(std::vector <SubtitleItem*> subtitles, grammarName name)
{
    bool generateQuickDict = false, generateQuickLM = false;

    if(name==quick_dict)
    {
        generateQuickDict = true;
        name = complete_grammar;
    }

    else if(name==quick_lm)
    {
        generateQuickLM = true;
        name = complete_grammar;
    }

    //create temporary directories in case they don't exist
    debugstream << "Creating temporary directories at tempFiles/";

#ifndef WIN32
    int rv = systemGetStatus("mkdir -p tempFiles/corpus tempFiles/dict tempFiles/vocab tempFiles/fsg tempFiles/lm");
#else
    int rv = systemGetStatus("if not exist tempFiles\\ mkdir tempFiles\\corpus tempFiles\\dict tempFiles\\vocab tempFiles\\fsg tempFiles\\lm");
#endif
  
    if (rv != 0)
    {
        fatalstream(EXIT_FAILURE) << "Unable to create directory tempFiles/ : " << strerror(errno);
    }


    debugstream << "Directories created successfully!";

    std::ofstream corpusDump, phoneticCorpusDump, fsgDump, vocabDump, dictDump, logDump;

    //setting exceptions to be thrown on failure
    corpusDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fsgDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    vocabDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    dictDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    phoneticCorpusDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    logDump.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        logDump.open("tempFiles/grammar.log",std::ios::binary);
        logDump.close();
    }

    catch(std::system_error& e)
    {
        fatalstream(EXIT_FAILURE) << e.code().message();
    }

    if(name == corpus || name == complete_grammar)
    {
		infostream << "Creating Corpus : tempFiles/corpus/corpus.txt";

        try
        {
            corpusDump.open("tempFiles/corpus/corpus.txt",std::ios::binary);
            corpusDump.close();
        }

        catch(std::system_error& e)
        {
            fatalstream(EXIT_FAILURE) << e.code().message();
        }

    }

    if(name == phone_lm || name == complete_grammar)
    {
        infostream << "Creating Phonetic Corpus : tempFiles/corpus/phoneticCorpus.txt";

        try
        {
            phoneticCorpusDump.open("tempFiles/corpus/phoneticCorpus.txt",std::ios::binary);
            phoneticCorpusDump.close();
        }

        catch(std::system_error& e)
        {
            fatalstream(EXIT_FAILURE) << e.code().message();
        }

    }

    for(SubtitleItem *sub : subtitles)
    {
        if(name == corpus || name == complete_grammar)
        {
            try
            {
                corpusDump.open("tempFiles/corpus/corpus.txt", std::ios::binary | std::ios::app);
            }

            catch(std::system_error& e)
            {
                fatalstream(EXIT_FAILURE) << e.code().message();
            }

            corpusDump << "<s> " << stringToLower(sub->getDialogue()) << " </s>\n";
            corpusDump.close();
        }

        if(name == phone_lm || name == complete_grammar)
        {
            try
            {
                phoneticCorpusDump.open("tempFiles/corpus/phoneticCorpus.txt", std::ios::binary | std::ios::app);
            }

            catch(std::system_error& e)
            {
                fatalstream(EXIT_FAILURE) << e.code().message();
            }

            int numberOfWords = sub->getWordCount();
            std::string printPhoneticCourpus = "SIL ";

            for(int i=0;i<numberOfWords;i++)
            {
                std::vector<Phoneme> phones = stringToPhoneme(stringToLower(sub->getWordByIndex(i)));

                for(Phoneme ph : phones)
                    printPhoneticCourpus += ph + " ";
            }

            printPhoneticCourpus += "SIL\n";

            phoneticCorpusDump << printPhoneticCourpus;
            phoneticCorpusDump.close();
        }

        if(name == fsg || name == complete_grammar)
        {
            long int startTime = sub->getStartTime();
            std::string fsgFileName("tempFiles/fsg/" + std::to_string(startTime));
            fsgFileName += ".fsg";

            try
            {
                fsgDump.open(fsgFileName, std::ios::binary);
            }

            catch(std::system_error& e)
            {
                fatalstream(EXIT_FAILURE) << e.code().message().c_str();
            }

            int numberOfWords = sub->getWordCount();

            fsgDump<<"FSG_BEGIN CUSTOM_FSG\n";
            fsgDump<<"NUM_STATES "<<(numberOfWords * 2) + 2<<"\n";
            fsgDump<<"START_STATE 0\n";
            fsgDump<<"FINAL_STATE "<<(numberOfWords * 2) + 1<<"\n\n";
            fsgDump<<"# Transitions\n";

            for(int i=0;i<numberOfWords;i++)
            {
                fsgDump<<"TRANSITION "<<"0 "<<i+1<<" 0.0909\n";
            }
            for(int i=numberOfWords, counter = 1;counter<=numberOfWords;i++,counter++)
            {
                fsgDump<<"TRANSITION "<<counter<<" "<<i+1<<" 1.0 "<<stringToLower(sub->getWordByIndex(counter-1))<<"\n";
            }
            for(int i=numberOfWords + 1;i<numberOfWords * 2 + 1;i++)
            {
                fsgDump<<"TRANSITION "<<i<<" "<<numberOfWords * 2 + 1<<" 0.0909\n";
            }

            fsgDump<<"TRANSITION "<<numberOfWords * 2 + 1<<" 0 0.0909\n";


            fsgDump<<"FSG_END\n";
            fsgDump.close();
        }

    }

    if(name == vocab || name == complete_grammar)
    {
		debugstream << "Creating vocabulary...";

        rv = systemGetStatus("text2wfreq < tempFiles/corpus/corpus.txt 2>tempFiles/grammar.log | wfreq2vocab > tempFiles/vocab/complete.vocab 2>tempFiles/grammar_wfreq2vocab.log");

        if (rv != 0)
        {
            fatalstream(EXIT_FAILURE) << "Something went wrong while creating vocabulary!";
        }

		debugstream << "Vocabulary created!";
    }

    if(name == dict || name == complete_grammar)
    {
        if(generateQuickDict)
        {
            std::ifstream vocabInput("tempFiles/vocab/complete.vocab");

            try
            {
                dictDump.open("tempFiles/dict/complete.dict", std::ios::binary);
            }

            catch(std::system_error& e)
            {
                fatalstream(EXIT_FAILURE) << e.code().message();
            }

            std::string word;
            std::vector<std::string> phonemes;
            bool beginWriting = false;

            while (std::getline(vocabInput, word))
            {
                if(beginWriting)
                {
                    phonemes = stringToPhoneme(word);

                    dictDump<<word<<" ";

                    for(std::string ph : phonemes)
                        dictDump<<ph<<" ";
                    dictDump<<"\n";
                }

                if(word == "<s>")
                    beginWriting = true;
            }

            vocabInput.close();
            dictDump.close();

        }

        else
        {
			infostream << "Creating the Dictionary, this might take a little time depending "
                "on your TensorFlow configuration : tempFiles/dict/complete.dict";

            rv = systemGetStatus("g2p-seq2seq --decode tempFiles/vocab/complete.vocab --model g2p-seq2seq-cmudict/ > tempFiles/dict/complete.dict");

            if (rv != 0)
            {
                fatalstream(EXIT_FAILURE) << "Something went wrong while creating dictionary!";
            }

			debugstream << "Dictionary created";
        }

    }

    if(name == lm || name == complete_grammar )
    {
        infostream << "Creating Biased Language Model : tempFiles/lm/complete.lm";

        if(generateQuickLM)
        {
            rv = systemGetStatus("perl quick_lm.pl -s tempFiles/corpus/corpus.txt 2>tempFiles/grammar.log");
            if (rv != 0)
            {
                fatalstream(EXIT_FAILURE) << "Something went wrong while creating Phonetic Language Model!";
            }
#ifndef WIN32
            rv = systemGetStatus("mv tempFiles/corpus/corpus.txt.arpabo tempFiles/lm/complete.lm 2>tempFiles/grammar.log");
#else
            rv = systemGetStatus("move tempFiles\\corpus\\corpus.txt.arpabo tempFiles\\lm\\complete.lm 2>tempFiles\\grammar.log");
#endif

            if (rv != 0)
            {
                fatalstream(EXIT_FAILURE) << "Something went wrong while moving phonetic model!";
            }
        }

        else
        {
            rv = systemGetStatus("text2idngram -vocab tempFiles/vocab/complete.vocab -idngram tempFiles/lm/lm.idngram <  tempFiles/corpus/corpus.txt 2>tempFiles/grammar.log");

            if (rv != 0)
            {
                fatalstream(EXIT_FAILURE) << "Something went wrong while creating idngram file!";
            }

            rv = systemGetStatus("idngram2lm -vocab_type 0 -idngram tempFiles/lm/lm.idngram -vocab tempFiles/vocab/complete.vocab  -arpa tempFiles/lm/complete.lm 2>tempFiles/grammar.log");

            if (rv != 0)
            {
                fatalstream(EXIT_FAILURE) << "Something went wrong while creating biased language model!";
            }
        }
		debugstream << "Biased Language Model created successfully!";

    }

    if(name == phone_lm  || name == complete_grammar )
    {
        infostream << "Creating Phonetic Language Model : tempFiles/lm/phone.lm";

        rv = systemGetStatus("perl quick_lm.pl -s tempFiles/corpus/phoneticCorpus.txt 2>tempFiles/grammar.log");
        if (rv != 0)
        {
            fatalstream(EXIT_FAILURE) << "Something went wrong while creating Phonetic Language Model!";
        }
#ifndef WIN32
        rv = systemGetStatus("mv tempFiles/corpus/phoneticCorpus.txt.arpabo tempFiles/lm/ 2>tempFiles/grammar.log");
#else
        rv = systemGetStatus("move tempFiles\\corpus\\phoneticCorpus.txt.arpabo tempFiles\\lm\\ 2>tempFiles\\grammar.log");
#endif
        if (rv != 0)
        {
            fatalstream(EXIT_FAILURE) << "Something went wrong while moving phonetic model!";
        }
    }

    debugstream << "Grammar files created!";

    return true;

}
