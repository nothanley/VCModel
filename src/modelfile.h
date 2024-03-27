/* Validates CAK file and initializes a stream upon validation success  */
#include <fstream>
#pragma once

namespace VCModel{

	class CPackage;
	class CContainer {

	public:
        CContainer();
        CContainer(std::istream* stream);
        CContainer(const char* path);
		~CContainer() 
		{
			if (m_pModel)
				delete m_pModel;
		}

		VCModel::CPackage* getModel() { return this->m_pModel; }
		int getVersion() { return this->m_fileVersion; }

	private:
		void Load();
		void ReadContents();
		void ValidateContainer();

	private:
		VCModel::CPackage* m_pModel = nullptr;
		std::string m_sFilePath;
		uint32_t m_fileVersion;
        std::istream* fs = nullptr;
//		uintptr_t m_fileSize;
		bool isOk = false;
	};

}
