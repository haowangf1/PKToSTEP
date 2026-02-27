
#ifndef __XCHG_MAIN_DOCUMENT_PTR_HPP__
#define __XCHG_MAIN_DOCUMENT_PTR_HPP__

class Xchg_MainDoc;
class Xchg_DocElement;
class Xchg_Node;
class Xchg_Component;
class Xchg_MetaData;
class Xchg_NodeConnector;
template <class T> class SmartPtr;

typedef SmartPtr<Xchg_MainDoc> Xchg_MainDocPtr;
typedef SmartPtr<Xchg_DocElement> Xchg_DocElementPtr;
typedef SmartPtr<Xchg_Node> Xchg_NodePtr;
typedef SmartPtr<Xchg_Component> Xchg_ComponentPtr;
typedef SmartPtr<Xchg_MetaData> Xchg_MetaDataPtr;
typedef SmartPtr<Xchg_NodeConnector> Xchg_NodeConnectorPtr;

typedef Xchg_ID Xchg_ComponentID;
typedef Xchg_ID Xchg_ComponentInstanceID;
typedef Xchg_ID Xchg_NodeID;

#endif