// CFW1TextGeometryInterface.cpp

#include "FW1Precompiled.h"

#include "CFW1TextGeometry.h"


namespace FW1FontWrapper {


// Query interface
HRESULT STDMETHODCALLTYPE CFW1TextGeometry::QueryInterface(REFIID riid, void **ppvObject) {
	if(ppvObject == NULL)
		return E_INVALIDARG;
	
	if(IsEqualIID(riid, __uuidof(IFW1TextGeometry))) {
		*ppvObject = static_cast<IFW1TextGeometry*>(this);
		AddRef();
		return S_OK;
	}
	
	return CFW1Object::QueryInterface(riid, ppvObject);
}


// Clear geometry
void STDMETHODCALLTYPE CFW1TextGeometry::Clear() {
	m_vertices.clear();
	m_maxSheetIndex = 0;
}


// Add a vertex
void STDMETHODCALLTYPE CFW1TextGeometry::AddGlyphVertex(const FW1_GLYPHVERTEX *pVertex) {
	m_vertices.push_back(*pVertex);
	
	UINT sheetIndex = pVertex->GlyphIndex >> 16;
	m_maxSheetIndex = std::max(m_maxSheetIndex, sheetIndex);
}


// Get current glyph vertices
FW1_VERTEXDATA STDMETHODCALLTYPE CFW1TextGeometry::GetGlyphVerticesTemp() {
	FW1_VERTEXDATA vertexData;
	
	if(!m_vertices.empty()) {
		// Sort and prepare vertices
		m_sortedVertices.resize(m_vertices.size());
		
		UINT sheetCount = m_maxSheetIndex+1;
		
		m_vertexCounts.resize(sheetCount);
		std::fill(m_vertexCounts.begin(), m_vertexCounts.end(), 0);
		
		for(size_t i=0; i < m_vertices.size(); ++i)
			++(m_vertexCounts[m_vertices[i].GlyphIndex >> 16]);
		
		m_vertexStartIndices.resize(sheetCount);
		m_vertexStartIndices[0] = 0;
		for(UINT i=1; i < sheetCount; ++i)
			m_vertexStartIndices[i] = m_vertexStartIndices[i-1] + m_vertexCounts[i-1];
		
		for(size_t i=0; i < m_vertices.size(); ++i) {
			const FW1_GLYPHVERTEX &vertex = m_vertices[i];
			const UINT sheetIndex = vertex.GlyphIndex >> 16;
			
			m_sortedVertices[m_vertexStartIndices[sheetIndex]] = vertex;
			m_sortedVertices[m_vertexStartIndices[sheetIndex]].GlyphIndex &= 0xffff;
			
			++(m_vertexStartIndices[sheetIndex]);
		}
		
		vertexData.SheetCount = sheetCount;
		vertexData.pVertexCounts = &m_vertexCounts[0];
		vertexData.TotalVertexCount = static_cast<UINT>(m_vertices.size());
		vertexData.pVertices = &m_sortedVertices[0];
	}
	else {
		vertexData.SheetCount = 0;
		vertexData.pVertexCounts = 0;
		vertexData.TotalVertexCount = 0;
		vertexData.pVertices = 0;
	}
	
	return vertexData;
}


}// namespace FW1FontWrapper
