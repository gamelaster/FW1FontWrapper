// CFW1FontWrapperInterface.cpp

#include "FW1Precompiled.h"

#include "CFW1FontWrapper.h"

#include "CFW1StateSaver.h"


namespace FW1FontWrapper {


// Query interface
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::QueryInterface(REFIID riid, void **ppvObject) {
	if(ppvObject == NULL)
		return E_INVALIDARG;
	
	if(IsEqualIID(riid, __uuidof(IFW1FontWrapper))) {
		*ppvObject = static_cast<IFW1FontWrapper*>(this);
		AddRef();
		return S_OK;
	}
	
	return CFW1Object::QueryInterface(riid, ppvObject);
}


// Get the factory that created this object
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetFactory(IFW1Factory **ppFactory) {
	if(ppFactory == NULL)
		return E_INVALIDARG;
	
	m_pFW1Factory->AddRef();
	*ppFactory = m_pFW1Factory;
	
	return S_OK;
}


// Get D3D11 device
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetDevice(ID3D11Device **ppDevice) {
	if(ppDevice == NULL)
		return E_INVALIDARG;
	
	m_pDevice->AddRef();
	*ppDevice = m_pDevice;
	
	return S_OK;
}


// Get DWrite factory
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetDWriteFactory(IDWriteFactory **ppDWriteFactory) {
	if(ppDWriteFactory == NULL)
		return E_INVALIDARG;
	
	m_pDWriteFactory->AddRef();
	*ppDWriteFactory = m_pDWriteFactory;
	
	return S_OK;
}


// Get glyph atlas
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetGlyphAtlas(IFW1GlyphAtlas **ppGlyphAtlas) {
	if(ppGlyphAtlas == NULL)
		return E_INVALIDARG;
	
	m_pGlyphAtlas->AddRef();
	*ppGlyphAtlas = m_pGlyphAtlas;
	
	return S_OK;
}


// Get glyph provider
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetGlyphProvider(IFW1GlyphProvider **ppGlyphProvider) {
	if(ppGlyphProvider == NULL)
		return E_INVALIDARG;
	
	m_pGlyphProvider->AddRef();
	*ppGlyphProvider = m_pGlyphProvider;
	
	return S_OK;
}


// Get render states
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetRenderStates(IFW1GlyphRenderStates **ppRenderStates) {
	if(ppRenderStates == NULL)
		return E_INVALIDARG;
	
	m_pGlyphRenderStates->AddRef();
	*ppRenderStates = m_pGlyphRenderStates;
	
	return S_OK;
}


// Get vertex drawer
HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetVertexDrawer(IFW1GlyphVertexDrawer **ppVertexDrawer) {
	if(ppVertexDrawer == NULL)
		return E_INVALIDARG;
	
	m_pGlyphVertexDrawer->AddRef();
	*ppVertexDrawer = m_pGlyphVertexDrawer;
	
	return S_OK;
}


// Draw text layout
void STDMETHODCALLTYPE CFW1FontWrapper::DrawTextLayout(
	ID3D11DeviceContext *pContext,
	IDWriteTextLayout *pTextLayout,
	FLOAT OriginX,
	FLOAT OriginY,
	UINT32 Color,
	UINT Flags
) {
	DrawTextLayout(pContext, pTextLayout, OriginX, OriginY, Color, NULL, NULL, Flags);
}


// Draw text layout
void STDMETHODCALLTYPE CFW1FontWrapper::DrawTextLayout(
	ID3D11DeviceContext *pContext,
	IDWriteTextLayout *pTextLayout,
	FLOAT OriginX,
	FLOAT OriginY,
	UINT32 Color,
	const FW1_RECTF *pClipRect,
	const FLOAT *pTransformMatrix,
	UINT Flags
) {
	// Get a text renderer
	IFW1TextRenderer *pTextRenderer = NULL;
	
	EnterCriticalSection(&m_textRenderersCriticalSection);
	if(!m_textRenderers.empty()) {
		pTextRenderer = m_textRenderers.top();
		m_textRenderers.pop();
	}
	LeaveCriticalSection(&m_textRenderersCriticalSection);
	
	if(pTextRenderer == NULL) {
		IFW1TextGeometry *pTextGeometry;
		HRESULT hResult = m_pFW1Factory->CreateTextGeometry(&pTextGeometry);
		if(FAILED(hResult)) {
		}
		else {
			IFW1TextRenderer *pNewTextRenderer;
			hResult = m_pFW1Factory->CreateTextRenderer(m_pGlyphProvider, pTextGeometry, &pNewTextRenderer);
			if(FAILED(hResult)) {
			}
			else {
				pTextRenderer = pNewTextRenderer;
			}
			
			pTextGeometry->Release();
		}
	}
	
	// Draw
	if(pTextRenderer != NULL) {
		HRESULT hResult = pTextRenderer->DrawTextLayout(pTextLayout, OriginX, OriginY, Color, Flags);
		if(SUCCEEDED(hResult) && (Flags & FW1_ANALYZEONLY) == 0) {
			// Flush the glyph atlas in case any new glyphs were added
			if((Flags & FW1_NOFLUSH) == 0)
				m_pGlyphAtlas->Flush(pContext);
			
			// Draw the vertices
			if((Flags & FW1_CACHEONLY) == 0) {
				IFW1TextGeometry *pTextGeometry;
				pTextRenderer->GetTextGeometry(&pTextGeometry);
				DrawGeometry(pContext, pTextGeometry, pClipRect, pTransformMatrix, Flags);
				pTextGeometry->Release();
			}
		}
		
		// Keep the text renderer for future use
		EnterCriticalSection(&m_textRenderersCriticalSection);
		m_textRenderers.push(pTextRenderer);
		LeaveCriticalSection(&m_textRenderersCriticalSection);
	}
}


// Draw text
void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
	ID3D11DeviceContext *pContext,
	const WCHAR *pszString,
	FLOAT FontSize,
	FLOAT X,
	FLOAT Y,
	UINT32 Color,
	UINT Flags
) {
	FW1_RECTF rect;
	
	rect.Left = rect.Right = X;
	rect.Top = rect.Bottom = Y;
	
	DrawString(pContext, pszString, NULL, FontSize, &rect, Color, NULL, NULL, Flags | FW1_NOWORDWRAP);
}


// Draw text
void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
	ID3D11DeviceContext *pContext,
	const WCHAR *pszString,
	const WCHAR *pszFontFamily,
	FLOAT FontSize,
	FLOAT X,
	FLOAT Y,
	UINT32 Color,
	UINT Flags
) {
	FW1_RECTF rect;
	
	rect.Left = rect.Right = X;
	rect.Top = rect.Bottom = Y;
	
	DrawString(pContext, pszString, pszFontFamily, FontSize, &rect, Color, NULL, NULL, Flags | FW1_NOWORDWRAP);
}


// Draw text
void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
	ID3D11DeviceContext *pContext,
	const WCHAR *pszString,
	const WCHAR *pszFontFamily,
	FLOAT FontSize,
	const FW1_RECTF *pFormatRect,
	UINT32 Color,
	const FW1_RECTF *pClipRect,
	const FLOAT *pTransformMatrix,
	UINT Flags
) {
	if(m_defaultTextInited) {
		UINT32 stringLength = 0;
		while(pszString[stringLength] != 0)
			++stringLength;
		
		if(stringLength > 0) {
			// Create DWrite text layout for the string
			IDWriteTextLayout *pTextLayout;
			HRESULT hResult = m_pDWriteFactory->CreateTextLayout(
				pszString,
				stringLength,
				m_pDefaultTextFormat,
				pFormatRect->Right - pFormatRect->Left,
				pFormatRect->Bottom - pFormatRect->Top,
				&pTextLayout
			);
			if(SUCCEEDED(hResult)) {
				// Layout settings
				DWRITE_TEXT_RANGE allText = {0, stringLength};
				pTextLayout->SetFontSize(FontSize, allText);
				
				if(pszFontFamily != NULL)
					pTextLayout->SetFontFamilyName(pszFontFamily, allText);
				
				if((Flags & FW1_NOWORDWRAP) != 0)
					pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				
				if(Flags & FW1_RIGHT)
					pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				else if(Flags & FW1_CENTER)
					pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				if(Flags & FW1_BOTTOM)
					pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
				else if(Flags & FW1_VCENTER)
					pTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				
				// Draw
				DrawTextLayout(
					pContext,
					pTextLayout,
					pFormatRect->Left,
					pFormatRect->Top,
					Color,
					pClipRect,
					pTransformMatrix,
					Flags
				);
				
				pTextLayout->Release();
			}
		}
	}
}


// Draw vertices
void STDMETHODCALLTYPE CFW1FontWrapper::DrawGeometry(
	ID3D11DeviceContext *pContext,
	IFW1TextGeometry *pGeometry,
	const FW1_RECTF *pClipRect,
	const FLOAT *pTransformMatrix,
	UINT Flags
) {
	FW1_VERTEXDATA vertexData = pGeometry->GetGlyphVerticesTemp();
	if(vertexData.TotalVertexCount > 0) {
		if(m_featureLevel < D3D_FEATURE_LEVEL_10_0 || m_pGlyphRenderStates->HasGeometryShader() == FALSE)
			Flags |= FW1_NOGEOMETRYSHADER;
		
		// Save state
		CFW1StateSaver stateSaver;
		bool restoreState = false;
		if((Flags & FW1_RESTORESTATE) != 0) {
			if(SUCCEEDED(stateSaver.saveCurrentState(pContext)))
				restoreState = true;
		}
		
		// Set shaders etc.
		if((Flags & FW1_STATEPREPARED) == 0)
			m_pGlyphRenderStates->SetStates(pContext, Flags);
		if((Flags & FW1_CONSTANTSPREPARED) == 0)
			m_pGlyphRenderStates->UpdateShaderConstants(pContext, pClipRect, pTransformMatrix);
		
		// Draw glyphs
		UINT temp = m_pGlyphVertexDrawer->DrawVertices(pContext, m_pGlyphAtlas, &vertexData, Flags, 0xffffffff);
		temp;
		
		// Restore state
		if(restoreState)
			stateSaver.restoreSavedState();
	}
}


// Flush the glyph atlas
void STDMETHODCALLTYPE CFW1FontWrapper::Flush(ID3D11DeviceContext *pContext) {
	m_pGlyphAtlas->Flush(pContext);
}


}// namespace FW1FontWrapper
