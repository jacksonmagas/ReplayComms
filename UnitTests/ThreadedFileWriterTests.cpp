#include "pch.h"
#include "CppUnitTest.h"
#include "../ReplayComms/ThreadedFileWriter.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudioWritingTests
{
	const std::filesystem::path testDir{
		std::filesystem::temp_directory_path()
        / "ReplayCommsTests"
	};

	#if defined(_WIN32) || defined(_WIN64)
	const std::filesystem::path nullDevice{"NUL"};
	#else
	const std::filesystem::path nullDevice{"/dev/null"};
	#endif

    TEST_CLASS(ThreadedFileWriterTests)
    {
    public:
        TEST_CLASS_INITIALIZE(MakeTestDir)
        {
            std::filesystem::create_directory(testDir);
        }

        TEST_CLASS_CLEANUP(RemoveTestDir)
        {
            //TODO turn on cleanup of test files
            //std::filesystem::remove_all(testDir);
        }
        TEST_METHOD(TestAddDataReturnsEmptyWhenFits)
        {
            // Arrange
            const size_t bufferSize = 1024;
            {
				ThreadedFileWriter writer(bufferSize, nullDevice);

				std::vector<std::byte> input(bufferSize, std::byte{42});
				std::span<std::byte> inputSpan(input);

				// Act
				auto remaining = writer.addData(inputSpan);


				// Assert
				Assert::AreEqual(size_t{0}, remaining.size(), L"All data should have been added to the buffer.");
            }
        }

        TEST_METHOD(TestAddDataReturnsRemainingWhenTooLarge)
        {
            // Arrange
            const size_t bufferSize = 10;
            {
				ThreadedFileWriter writer(bufferSize, nullDevice);

				std::vector<std::byte> input(20, std::byte{1});
				std::span<std::byte> inputSpan(input);

				// Act
				auto remaining = writer.addData(inputSpan);

				// Assert
				Assert::AreEqual(size_t{10}, remaining.size(), L"Only part of the data should fit in the buffer.");
            }
        }

        TEST_METHOD(TestAddDataWithEmptySpan)
        {
            // Arrange
            const size_t bufferSize = 10;
            {
				ThreadedFileWriter writer(bufferSize, nullDevice);

				std::vector<std::byte> input;

				// Act
				auto remaining = writer.addData(input);

                using namespace std::chrono_literals;

				// Assert
				Assert::AreEqual(size_t{0}, remaining.size(), L"Empty input should return empty remaining span.");
            }
        }
		TEST_METHOD(TestAddDataActuallyWritesToFile)
		{
			// Arrange
			const size_t bufferSize = 10;
			const std::string fileName = "TestAddDataActuallyWrites.bin";
			const std::vector<std::byte> input{ std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4} };

			std::filesystem::path filePath = testDir / fileName;

			// Ensure file doesn't exist before test
			std::filesystem::remove(filePath);

			// Act
			{
				ThreadedFileWriter writer(bufferSize, filePath);
				auto remaining = writer.addData(input);

				// Assert: addData returns empty remaining
				Assert::AreEqual(size_t{0}, remaining.size(), L"All data should fit into the buffer.");
			} // writer goes out of scope, write thread should flush remaining data

			// Assert: file exists and contains the correct data
			Assert::IsTrue(std::filesystem::exists(filePath), L"File should exist after writer is destroyed.");

			std::ifstream in(filePath, std::ios::binary);
			Assert::IsTrue(in.is_open(), L"File should be readable.");

			std::vector<char> fileData((std::istreambuf_iterator<char>(in)),
											 std::istreambuf_iterator<char>());

			Assert::AreEqual(input.size(), fileData.size(), L"File should contain same number of bytes as input.");
			for (size_t i = 0; i < input.size(); ++i)
			{
				Assert::AreEqual(static_cast<char>(input[i]),
								 static_cast<char>(fileData[i]),
								 L"File data should match input.");
			}
		}
    };
}
