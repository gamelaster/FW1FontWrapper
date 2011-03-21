// CFW1TextGeometry.h

#ifndef IncludeGuard__FW1_CFW1TextGeometry
#define IncludeGuard__FW1_CFW1TextGeometry

#include "CFW1Object.h"


namespace FW1FontWrapper {


// Vector of vertices with sorting per glyph sheet
class CFW1TextGeometry : public CFW1Object<IFW1TextGeometry> {
	public:
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
		
		// IFW1TextGeometry
		virtual void STDMETHODCALLTYPE Clear();
		virtual void STDMETHODCALLTYPE AddGlyphVertex(const FW1_GLYPHVERTEX *pVertex);
		
		virtual FW1_VERTEXDATA STDMETHODCALLTYPE GetGlyphVerticesTemp();
	
	// Public functions
	public:
		CFW1TextGeometry();
		
		HRESULT initTextGeometry(IFW1Factory *pFW1Factory);
	
	// Internal functions
	protected:
		CFW1TextGeometry(const CFW1TextGeometry&);
		CFW1TextGeometry& operator=(const CFW1TextGeometry&);
		
		virtual ~CFW1TextGeometry();
	
	// Internal data
	protected:
		std::vector<FW1_GLYPHVERTEX>	m_vertices;
		std::vector<FW1_GLYPHVERTEX>	m_sortedVertices;
		
		UINT							m_maxSheetIndex;
		std::vector<UINT>				m_vertexCounts;
		std::vector<UINT>				m_vertexStartIndices;
};


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1TextGeometry
