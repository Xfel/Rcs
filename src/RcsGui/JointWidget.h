/*******************************************************************************

  Copyright (c) 2017, Honda Research Institute Europe GmbH.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. All advertising materials mentioning features or use of this software
     must display the following acknowledgement: This product includes
     software developed by the Honda Research Institute Europe GmbH.

  4. Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#ifndef JOINTWIDGET_H
#define JOINTWIDGET_H

#include <Rcs_graph.h>

#include <QScrollArea>
#include <QLCDNumber>
#include <QCheckBox>

#include <pthread.h>

class JointSlider;

namespace Rcs
{

class JointWidget: public QScrollArea
{
  Q_OBJECT

public:
  static int create(const RcsGraph* graph,
                    pthread_mutex_t* graphLock = NULL,
                    MatNd* q_des = NULL,
                    MatNd* q_curr = NULL,
                    bool alwaysWriteToQ = false,
                    bool passive = false);

  static int create(RcsGraph* graph,
                    pthread_mutex_t* graphLock = NULL,
                    MatNd* q_des = NULL,
                    MatNd* q_curr = NULL,
                    bool alwaysWriteToQ = false,
                    bool passive = false);

  static bool destroy(int handle);
  void reset(const MatNd* q);

  class JointChangeCallback
  {
  public:
    virtual void callback() = 0;
  };

  void registerCallback(JointChangeCallback* callback);

private slots:

  void toggle(void);
  void setJoint(void);
  void setConstraint(void);
  void displayAct(void);

private:

  static void* stateGui(void* arg);

  JointWidget(RcsGraph* graph, const RcsGraph* constGraph,
              pthread_mutex_t* graphLock = NULL,
              MatNd* q_des = NULL, MatNd* q_curr = NULL,
              bool alwaysWriteToQ = false,
              bool passive = false);
  ~JointWidget();

  void lock() const;
  void unlock() const;
  RcsGraph* _graph;
  const RcsGraph* _constGraph;
  int _togglestate;
  bool _alwaysWriteToQ;
  MatNd* _q_des;
  MatNd* _q_curr;
  QLCDNumber** lcd_q_cmd;
  QLCDNumber** lcd_q_act;
  JointSlider** jsc_q;
  QCheckBox** check_constraints;
  mutable pthread_mutex_t* mutex;
  std::vector<JointChangeCallback*> callback;
};

}   // namespace Rcs

#endif   // JOINTWIDGET_H
