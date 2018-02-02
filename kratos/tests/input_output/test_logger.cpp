//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics 
//
//  License:		 BSD License 
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//                   
//


// Project includes
#include "testing/testing.h"
#include "input_output/logger.h"
#include "input_output/logger_table_output.h"


namespace Kratos {
	namespace Testing {

		KRATOS_TEST_CASE_IN_SUITE(LoggerMessageStream, KratosCoreFastSuite)
		{
			LoggerMessage message("label");

			message << "Test message with number " << 12 << 'e' << "00";

			KRATOS_CHECK_C_STRING_EQUAL(message.GetLabel().c_str(), "label");
			KRATOS_CHECK_C_STRING_EQUAL(message.GetMessage().c_str(), "Test message with number 12e00");
			KRATOS_CHECK_EQUAL(message.GetSeverity(), LoggerMessage::Severity::INFO);
            KRATOS_CHECK_EQUAL(message.GetCategory(), LoggerMessage::Category::STATUS);
            KRATOS_CHECK_EQUAL(message.GetLocation().GetFileName(), "Unknown");
            KRATOS_CHECK_EQUAL(message.GetLocation().GetFunctionName(), "Unknown");
            KRATOS_CHECK_EQUAL(message.GetLocation().GetLineNumber(), -1);

			message << LoggerMessage::Severity::DETAIL 
                << LoggerMessage::Category::CRITICAL 
                << KRATOS_CODE_LOCATION << std::endl;

			KRATOS_CHECK_C_STRING_EQUAL(message.GetMessage().c_str(), "Test message with number 12e00\n");
			KRATOS_CHECK_EQUAL(message.GetSeverity(), LoggerMessage::Severity::DETAIL);
            KRATOS_CHECK_EQUAL(message.GetCategory(), LoggerMessage::Category::CRITICAL);
            KRATOS_CHECK_NOT_EQUAL(message.GetLocation().GetFileName().find("test_logger.cpp"), std::string::npos);
            KRATOS_CHECK_EQUAL(message.GetLocation().GetFunctionName(), "virtual void Kratos::Testing::TestLoggerMessageStream::TestFunction()");
            KRATOS_CHECK_EQUAL(message.GetLocation().GetLineNumber(), 40);
		}

		KRATOS_TEST_CASE_IN_SUITE(LoggerOutput, KratosCoreFastSuite)
		{
			std::stringstream buffer;
			LoggerOutput output(buffer);

			LoggerMessage message("label");
			message << "Test message with number " << 12 << 'e' << "00";

			output.WriteMessage(message);
			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test message with number 12e00");
		}

		KRATOS_TEST_CASE_IN_SUITE(LoggerStream, KratosCoreFastSuite)
		{
			static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

			Logger("TestLabel") << "Test message with number " << 12 << 'e' << "00";

			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test message with number 12e00");

			Logger("TestDetail") << Logger::Severity::DETAIL << "This log has detailed severity and will not be printed in output " 
				<< Logger::Category::CRITICAL << std::endl;

			// The message has DETAIL severity and should not be written
			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test message with number 12e00");
        }

		KRATOS_TEST_CASE_IN_SUITE(CheckPoint, KratosCoreFastSuite)
		{
			std::stringstream buffer;
			LoggerOutput output(buffer);

			KRATOS_CHECK_POINT("TestCheckPoint") << "The value in check point is " << 3.14;

#if defined(KRATOS_ENABLE_CHECK_POINT)
			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "The value in check point is 3.14");
#else
			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), ""); // should print noting
#endif
		}

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamInfo, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

			KRATOS_INFO("TestInfo") << "Test info message";

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test info message");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamInfoIf, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

            KRATOS_INFO_IF("TestInfo", true) << "Test info message";
            KRATOS_INFO_IF("TestInfo", false) << "This should not appear";

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test info message");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamInfoOnce, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

			for(std::size_t i = 0; i < 10; i++) {
                KRATOS_INFO_ONCE("TestInfo") << "Test info message - " << i;
            }

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test info message - 0");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamInfoFirst, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

            for(std::size_t i = 0; i < 10; i++) {
                KRATOS_INFO_FIRST_N("TestInfo", 4) << ".";
            }

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "....");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamWarning, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

			KRATOS_INFO("TestWarning") << "Test info message";

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test info message");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamWarningIf, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

            KRATOS_INFO_IF("TestWarning", true) << "Test warning message";
            KRATOS_INFO_IF("TestWarning", false) << "This should not appear";

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test warning message");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamWarningOnce, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

			for(std::size_t i = 0; i < 10; i++) {
                KRATOS_INFO_ONCE("TestWarning") << "Test warning message - " << i;
            }

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Test warning message - 0");
        }

        KRATOS_TEST_CASE_IN_SUITE(LoggerStreamWarningFirst, KratosCoreFastSuite)
        {
            static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerOutput(buffer));
			Logger::AddOutput(p_output);

            for(std::size_t i = 0; i < 10; i++) {
                KRATOS_INFO_FIRST_N("TestWarning", 4) << ".";
            }

            KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "....");
        }


		KRATOS_TEST_CASE_IN_SUITE(LoggerTableOutput, KratosCoreFastSuite)
		{
			static std::stringstream buffer;
			LoggerOutput::Pointer p_output(new LoggerTableOutput(buffer, {"Time Step    ", "Iteration Number", "Convergence"}));
			Logger::AddOutput(p_output);
            p_output->WriteHeader();

			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Time Step     Iteration Number Convergence \n");

            std::size_t time_step = 1;
			Logger("Time Step") << time_step;

			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Time Step     Iteration Number Convergence \n1");

			Logger("Label") << "This log has a lable which is not in the output columns and will not be printed in output " ;

			KRATOS_CHECK_C_STRING_EQUAL(buffer.str().c_str(), "Time Step     Iteration Number Convergence \n1");

            std::cout << buffer.str() << std::endl;

        }

	}   // namespace Testing
}  // namespace Kratos.


