/*
  Research carried out within the scope of the Associated International Laboratory: Joint Japanese-French Robotics Laboratory (JRL)

  Developed by Florent Lamiraux (LAAS-CNRS)

*/

#include <iostream>

#include "KineoKCDModel/kppKCDBox.h"
#include "hppCorbaServer/hppciServer.h"
#include "hppCorbaServer/hppciObstacle.h"

#include "hppCorbaServer/hppciTools.h"

#if DEBUG==2
#define ODEBUG2(x) std::cout << "ChppciRobot:" << x << std::endl
#define ODEBUG1(x) std::cerr << "ChppciRobot:" << x << std::endl
#elif DEBUG==1
#define ODEBUG2(x)
#define ODEBUG1(x) std::cerr << "ChppciRobot:" << x << std::endl
#else
#define ODEBUG2(x)
#define ODEBUG1(x)
#endif

// ==========================================================================

ChppciObstacle_impl::ChppciObstacle_impl(ChppciServer *inHppciServer) : 
  attHppPlanner(inHppciServer->getHppPlanner()), attHppciServer(inHppciServer)
{
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::setObstacles(const char* inListName)
{
  std::string listName(inListName);

  // Check that collision list exists.
  if (collisionListMap.count(listName) != 1) {
    ODEBUG1("collision list " << listName << " does not exist.");
    return -1;
  }
  std::string name(listName);
  std::vector<CkcdObjectShPtr> kcdCollisionList = collisionListMap[listName];
  // CkcdCollisionListShPtr kcdCollisionList = collisionListMap[name];
  attHppPlanner->obstacleList(kcdCollisionList);
  return 0;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::addObstacle(const char* inPolyhedronName)
{
  std::string polyhedronName(inPolyhedronName);

  // Check that polyhedron exists.
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr hppPolyhedron = polyhedronMap[polyhedronName];

  // Build collision entity for KCD.
  hppPolyhedron->makeCollisionEntity();
  attHppPlanner->addObstacle(hppPolyhedron);
  return 0;  
}

CORBA::Short ChppciObstacle_impl::addObstacleConfig(const char* inPolyName, 
						    const hppCorbaServer::Configuration& cfg)
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyName);

  // Check that polyhedron exists.
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];

  // Build collision entity for KCD.
  polyhedron->makeCollisionEntity();

  CkitMat4 mat;
  ConfigurationToCkitMat4(cfg, mat);
  
  polyhedron->setAbsolutePosition(mat);
  attHppPlanner->addObstacle(polyhedron);
  return 0;  
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::moveObstacleConfig(const char* inPolyName, 
						    const hppCorbaServer::Configuration& cfg)
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyName);

  // get the obstacle list from the planner.
  std::vector< CkcdObjectShPtr > obstList = attHppPlanner->obstacleList();

  // get the hppPolyhedron
  bool findFlag = false;
  CkppKCDPolyhedronShPtr poly;
  for(unsigned int i=0; i<obstList.size(); i++){
    if(poly =  boost::dynamic_pointer_cast<CkppKCDPolyhedron>(obstList[i])){
      if(polyhedronName == poly->name()){
	findFlag=true;
	ODEBUG2(" obstacle "<<polyhedronName<<" found in the tree.");
	break;
      }
    }
    else{
      ODEBUG1("only  CkppKCDPolyhedron is supported.");
    }
  }

  // if there is, change its configuration.
  if(findFlag){
    CkitMat4 mat;
    ConfigurationToCkitMat4(cfg, mat);

    poly->setAbsolutePosition(mat);
  
    return 0;  
  }

  else
    return -1;
}


// ==========================================================================

CORBA::Short ChppciObstacle_impl::createCollisionList(const char* inListName)
  throw(CORBA::SystemException)
{
  std::string listName(inListName);

  // Check that collision list does not already exist.
  if (collisionListMap.count(listName) != 0) {
    ODEBUG1("collision list " << listName << " already exists.");
    return -1;
  }
  std::vector<CkcdObjectShPtr> collisionList ;
  collisionListMap[listName] = collisionList;
  return 0;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::addPolyToCollList(const char* inListName, 
						    const char* inPolyhedronName)
  throw(CORBA::SystemException)
{
  std::string listName(inListName);
  std::string polyhedronName(inPolyhedronName);

  // Check that collision list exists.
  if (collisionListMap.count(listName) != 1) {
    ODEBUG1("collision list " << listName << " does not exist.");
    return -1;
  }
#if DEBUG==2
  std::cout<<" in ChppciObstacle_impl::addPolyToCollList() polyhedronMap size "
	   << polyhedronMap.size() << std::endl;
  std::map<std::string, CkppKCDPolyhedronShPtr>::iterator it = polyhedronMap.begin();
  for(; it != polyhedronMap.end(); it++) {
    cout<<" name: "<< it->first << endl;
  }
#endif
  // Check that polyhedron exists.
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  std::vector<CkcdObjectShPtr>& kcdCollisionList = collisionListMap[listName];
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];

  // Build collision entity for KCD.
  polyhedron->makeCollisionEntity();

  kcdCollisionList.push_back(polyhedron);

  return 0;
}

// ==========================================================================
 
CORBA::Short ChppciObstacle_impl::createPolyhedron(const char* inPolyhedronName)
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyhedronName);
  // Check that polyhedron does not already exist.
  if (polyhedronMap.count(polyhedronName) != 0) {
    ODEBUG1("polyhedron " << polyhedronName << " already exists.");
    return -1;
  }
  CkppKCDPolyhedronShPtr hppPolyhedron = CkppKCDPolyhedron::create(polyhedronName);

  if (!hppPolyhedron) {
    ODEBUG1("failed to create polyhedron " << polyhedronName);
    return -1;
  }
  polyhedronMap[polyhedronName] = hppPolyhedron;

  return 0;
}

// ==========================================================================
 
CORBA::Short ChppciObstacle_impl::createBox(const char* inBoxName, 
					    CORBA::Double x, CORBA::Double y, 
					    CORBA::Double z)
  throw(CORBA::SystemException)
{
  //  attHppciServer->waitForMutex(std::string("ChppciObstacle_impl::createBox"));
  std::string polyhedronName(inBoxName);
  // Check that polyhedron does not already exist.
  if (polyhedronMap.count(polyhedronName) != 0) {
    ODEBUG1("polyhedron " << polyhedronName << " already exists.");
    return -1;
  }
  CkppKCDPolyhedronShPtr hppPolyhedron = CkppKCDBox::create(polyhedronName, x, y, z);

  if (!hppPolyhedron) {
    ODEBUG1("failed to create polyhedron " << polyhedronName);
    return -1;
  }
  polyhedronMap[polyhedronName] = hppPolyhedron;

  return 0;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::addPoint(const char* inPolyhedronName, 
					CORBA::Double x, CORBA::Double y, 
					CORBA::Double z) 
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyhedronName);
#if DEBUG==2
  // Check that polyhedron exists.
  std::cout<<" in ChppciObstacle_impl::addPoint() polyhedronMap size "<<polyhedronMap.size()<<std::endl;
  std::map<std::string, CkppKCDPolyhedronShPtr>::iterator it = polyhedronMap.begin();
  for(; it != polyhedronMap.end(); it++) {
    cout<<" name: "<<it->first<<endl;
  }
#endif
  // Check that polyhedron exists.
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];
  unsigned int rank;

  /*
  CkitPoint3 point(x, y, z);
  CkcdPolyhedronShPtr poly = KIT_DYNAMIC_PTR_CAST(CkcdPolyhedron, hppPolyhedron);
  */

  polyhedron->CkcdPolyhedron::addPoint(x, y, z, rank);

  return rank;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::addTriangle(const char* inPolyhedronName, 
					      long pt1, long pt2, long pt3)
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyhedronName);
#if DEBUG==2
  // Check that polyhedron exists.
  std::cout<<" in ChppciObstacle_impl::addTriangle() polyhedronMap size "<<polyhedronMap.size()<<std::endl;
  std::map<std::string, CkppKCDPolyhedronShPtr>::iterator it = polyhedronMap.begin();
  for(; it != polyhedronMap.end(); it++) {
    cout<<" name: "<<it->first<<endl;
  }
#endif

  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];
  unsigned int rank;

  polyhedron->addTriangle(pt1, pt2, pt3, rank);

  return rank;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::setVisible(const char* inPolyname, 
					     CORBA::Boolean inVisible)
    throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyname);
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];
  polyhedron->isVisible(inVisible);

  return 0;
}

// ==========================================================================

CORBA::Short ChppciObstacle_impl::setTransparent(const char* inPolyname, 
						 CORBA::Boolean inTransparent)
  throw(CORBA::SystemException)
{
  std::string polyhedronName(inPolyname);
  if (polyhedronMap.count(polyhedronName) != 1) {
    ODEBUG1("polyhedron " << polyhedronName << " does not exist.");
    return -1;
  }
  CkppKCDPolyhedronShPtr polyhedron = polyhedronMap[polyhedronName];
  polyhedron->isTransparent(inTransparent);

  return 0;
}

