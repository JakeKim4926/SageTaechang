
// SageTaechangView.cpp: CSageTaechangView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "SageTaechang.h"
#endif

#include "SageTaechangDoc.h"
#include "SageTaechangView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSageTaechangView

IMPLEMENT_DYNCREATE(CSageTaechangView, CView)

BEGIN_MESSAGE_MAP(CSageTaechangView, CView)
END_MESSAGE_MAP()

// CSageTaechangView 생성/소멸

CSageTaechangView::CSageTaechangView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CSageTaechangView::~CSageTaechangView()
{
}

BOOL CSageTaechangView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CSageTaechangView 그리기

void CSageTaechangView::OnDraw(CDC* /*pDC*/)
{
	CSageTaechangDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}


// CSageTaechangView 진단

#ifdef _DEBUG
void CSageTaechangView::AssertValid() const
{
	CView::AssertValid();
}

void CSageTaechangView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSageTaechangDoc* CSageTaechangView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSageTaechangDoc)));
	return (CSageTaechangDoc*)m_pDocument;
}
#endif //_DEBUG


// CSageTaechangView 메시지 처리기
