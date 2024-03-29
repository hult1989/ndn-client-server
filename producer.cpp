/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
* Copyright (c) 2013-2015 Regents of the University of California.
*
* This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
*
* ndn-cxx library is free software: you can redistribute it and/or modify it under the
* terms of the GNU Lesser General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later version.
*
* ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
* PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
*
* You should have received copies of the GNU General Public License and GNU Lesser
* General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
* <http://www.gnu.org/licenses/>.
*
* See AUTHORS.md for complete list of ndn-cxx authors and contributors.
*
* @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
*/

// correct way to include ndn-cxx headers
// #include <ndn-cxx/face.hpp>
// #include <ndn-cxx/security/key-chain.hpp>

#include "face.hpp"
#include "security/key-chain.hpp"
#include<time.h>
#include<cstdlib>
#include <ndn-cxx/util/time.hpp>
#include <sys/time.h>


int PACKET_SIZE = 1;

static uint8_t content[1024*1024];
long totalTime = 0;
timeval start, end;
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class Producer : noncopyable
{
public:
void
run()
{
	m_face.setInterestFilter("/example/a",
							bind(&Producer::onInterestB, this, _1, _2),
							RegisterPrefixSuccessCallback(),
							bind(&Producer::onRegisterFailed, this, _1, _2));
	m_face.setInterestFilter("/example/b",
							bind(&Producer::onInterestB, this, _1, _2),
							RegisterPrefixSuccessCallback(),
							bind(&Producer::onRegisterFailed, this, _1, _2));
	
		m_face.setInterestFilter("/example/testApp",
							bind(&Producer::onInterestB, this, _1, _2),
							RegisterPrefixSuccessCallback(),
							bind(&Producer::onRegisterFailed, this, _1, _2));
	m_face.processEvents();
	
	std::cout <<"total time on signature is:\t" << totalTime / 1000 << " ms"<< std::endl;
}

private:
void
onInterestA(const InterestFilter& filter, const Interest& interest)
{
// 		std::cout << "<< I: " << interest << std::endl;

	// Create new name, based on Interest's name
	Name dataName(interest.getName());
// 		dataName.appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)
// 		.append("testApp") // add "testApp" component to Interest name


	// Create Data packet
	shared_ptr<Data> data = make_shared<Data>();
	data->setName(dataName);
	data->setFreshnessPeriod(time::seconds(100));
	data->setContent(content, PACKET_SIZE * 1024);

	gettimeofday(&start, 0);
 	m_keyChain.sign(*data);
//	m_keyChain.signWithSha256(*data);
	// m_keyChain.sign(data, <identityName>);
	// m_keyChain.sign(data, <certificate>);

	// Return Data packet to the requester
// 		std::cout << ">> D: " << *data << std::endl;
	gettimeofday(&end, 0);
	totalTime += (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;


	m_face.put(*data);
}

private:
void
onInterestB(const InterestFilter& filter, const Interest& interest)
{
// 		std::cout << "<< I: " << interest << std::endl;

	// Create new name, based on Interest's name
	Name dataName(interest.getName());
// 		dataName.appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)
// 		.append("testApp") // add "testApp" component to Interest name


	// Create Data packet
	shared_ptr<Data> data = make_shared<Data>();
	data->setName(dataName);
	data->setFreshnessPeriod(time::seconds(10));
	data->setContent(content, PACKET_SIZE * 1024);

	gettimeofday(&start, 0);
// 	m_keyChain.sign(*data);
	m_keyChain.signWithSha256(*data);
	// m_keyChain.sign(data, <identityName>);
	// m_keyChain.sign(data, <certificate>);

	// Return Data packet to the requester
// 		std::cout << ">> D: " << *data << std::endl;
	gettimeofday(&end, 0);
	totalTime += (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;


	m_face.put(*data);
}


private:
void
onInterestC(const InterestFilter& filter, const Interest& interest)
{
// 		std::cout << "<< I: " << interest << std::endl;

	// Create new name, based on Interest's name
	Name dataName(interest.getName());
// 		dataName.appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)
// 		.append("testApp") // add "testApp" component to Interest name


	// Create Data packet
	shared_ptr<Data> data = make_shared<Data>();
	data->setName(dataName);
	data->setFreshnessPeriod(time::seconds(10));
	data->setContent(content, PACKET_SIZE * 1024);

	gettimeofday(&start, 0);
 	m_keyChain.sign(*data);
//	m_keyChain.signWithSha256(*data);
	// m_keyChain.sign(data, <identityName>);
	// m_keyChain.sign(data, <certificate>);

	// Return Data packet to the requester
// 		std::cout << ">> D: " << *data << std::endl;
	gettimeofday(&end, 0);
	totalTime += (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;


	m_face.put(*data);
}



void
onRegisterFailed(const Name& prefix, const std::string& reason)
{
	std::cerr << "ERROR: Failed to register prefix \""
			<< prefix << "\" in local hub's daemon (" << reason << ")"
			<< std::endl;
	m_face.shutdown();
}

private:
Face m_face;
KeyChain m_keyChain;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
	PACKET_SIZE = atoi(*(argv + 1));
	int total = 1024 * 1024;
	for(int i = 0; i < total; i++)
		content[i] = (uint8_t)('x');
	ndn::examples::Producer producer;
	try {
		producer.run();
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
	};
	return 0;
}
