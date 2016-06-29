#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <qwidget.h>
#include <qstring.h>
#include <qslider.h>   
#include <qcheckbox.h>  
#include <qlabel.h>


#include <qspinbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qpainter.h>
#include <alsa/asoundlib.h>
#include "synthdata.h"
#include "m_vca.h"

#include "thread_pool.hpp"
#include <future>
#include <functional>

M_vca::M_vca(bool p_expMode, QWidget* parent) 
: Module(M_type_vca, 1, parent, p_expMode ? tr("Exp. VCA") : tr("Lin. VCA"))
{
	setGeometry(MODULE_NEW_X, MODULE_NEW_Y, MODULE_VCA_WIDTH, MODULE_VCA_HEIGHT);
	gain1 = 0;
	gain2 = 0;
	in1 = 1.0;
	in2 = 1.0;
	out = 1.0;
	expMode = p_expMode;
	port_M_gain1 = new Port(tr("Gain 0"), PORT_IN, 0, this);
	inPortList.push_back(port_M_gain1);
	port_M_gain2 = new Port(tr("Gain 1"), PORT_IN, 1, this);
	inPortList.push_back(port_M_gain2);
	port_M_in1 = new Port(tr("In 0"), PORT_IN, 2, this);
	inPortList.push_back(port_M_in1);
	port_M_in2 = new Port(tr("In 1"), PORT_IN, 3, this);
	inPortList.push_back(port_M_in2);
	cv.out_off = 115;
	port_out = new Port(tr("Out"), PORT_OUT, 0, this);

	configDialog->addSlider(tr("Gain"), gain1, 0, 1);
	configDialog->addSlider(tr("Gain 1"), gain2, 0, 1);
	configDialog->addSlider(tr("In 0"), in1, 0, 2);
	configDialog->addSlider(tr("In 1"), in2, 0, 2);
	configDialog->addSlider(tr("Output level"), out, 0, 2);

	totalInPorts = 4;
	inDataVector.resize(totalInPorts);
	gainData1 = port_M_gain1->inputData;
	this->inDataVector.push_back(port_M_gain1->inputData);
	gainData2 = port_M_gain2->inputData;
	this->inDataVector.push_back(port_M_gain2->inputData);
	inData1 = port_M_in1->inputData;
	this->inDataVector.push_back(port_M_in1->inputData);
	inData2 = port_M_in2->inputData;
	this->inDataVector.push_back(port_M_in2->inputData);

	futures_vector.resize(totalInPorts);
	k_futures.resize(totalInPorts, 99);//set dummy value for empty slots
}

void M_vca::generateCycle() {

	int l1;
	unsigned int l2;
	float  v, g;

	//////////////  Paralleliztion starts here ////////////////////

	int i = 0;
	int j = 0;
	int k = 0;
	bool fut = false;


    for (it = inPortList.begin() ; it != inPortList.end(); ++it){

	 if((*it)->connectedPortList.size()>0){
	   for ( jt = inPortList.begin(); jt != inPortList.end(); ++jt){

		if((i!=j)&&((*jt)->connectedPortList.size()>0)&&(synthdata->running_futures < synthdata->max_futures)
					&&((*it)->connectedPortList.at(0)->module->moduleID !=(*jt)->connectedPortList.at(0)->module->moduleID))
		{
		//parallel branch sent to threadpool
		Port *next;
		next = (*it);

                std::function<float**()> task = std::bind(&Port::getinputdata, &*next);
		futures_vector[k] = pool->run_task(task);
		k_futures[k] = i;
		k++;
	        synthdata->running_futures++;
		fut = true;
		break;//return to i

		}

  	else { //inport is connected but to the same out port
	     
	     this->inDataVector.at(i) = this->inPortList.at(i)->getinputdata();//maybe only connection
             j++;
        }

     } else {
	   
	    this->inDataVector.at(i)= inPortList.at(i)->getinputdata();//called on empty ports

	}

	   i++;

  }//end outer loop

    if(fut){ //get futures from thread pool
	int k2 = 0;
	 for(int i = 0;i<totalInPorts; i++){
           if(i == k_futures[k2]){
		this->inPortList.at(i)->inputData = futures_vector[k2].get();
		this->inDataVector.at(i) = this->inPortList.at(i)->inputData;
		k2++;
		  }
	       }
          }
    } else {
	for (it = inPortList.begin() ; it != inPortList.end(); ++it){
		this->inDataVector.at(i) = this->inPortList.at(i)->getinputdata();
		i++;
	   }
	}
	gainData1 = this->inDataVector.at(0);//port_M_gain1->inputData;
	gainData2 = this->inDataVector.at(1);//port_M_gain2->inputData;
	inData1 = this->inDataVector.at(2);//port_M_in1->inputData;
	inData2 = this->inDataVector.at(3); //port_M_in2->inputData;

 ////////////////////////////// input data returned here end of parallel search calculate data ///////////////////////////////

	if (expMode) {
		for (l1 = 0; l1 < synthdata->poly; l1++) {
			for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
				v = gain1 + gainData1[l1][l2] + gain2 * gainData2[l1][l2];
				g = (v > 0) ? synthdata->exp_table ((v - 1.0) * 9.21) : 0;  // This gives a range of 80 dB
				data[0][l1][l2] = g * out * (in1 * inData1[l1][l2] + in2 * inData2[l1][l2]);
			}
		}
	} else {
		for (l1 = 0; l1 < synthdata->poly; l1++) {
			for (l2 = 0; l2 < synthdata->cyclesize; l2++) {
				data[0][l1][l2] = (gain1 + gainData1[l1][l2] + gain2 * gainData2[l1][l2])
                        								  * out * (in1 * inData1[l1][l2] + in2 * inData2[l1][l2]);
			}
		}
	}
}



