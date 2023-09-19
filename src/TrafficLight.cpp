#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <iostream>
#include <deque>
#include <mutex>
#include <condition_variable>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this] { return !_queue.empty(); });
        T msg = std::move(_queue.front());
        _queue.pop_front();
        return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
        _queue.emplace_back(std::move(msg));
        _condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : 
    while (true) {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread(&TrafficLight::cycleThroughPhases, this).detach();
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    // Random number generator for cycle duration between 4 and 6 seconds
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000); // In milliseconds

    int cycleDuration = distribution(gen);

    auto lastUpdate = std::chrono::system_clock::now();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Sleep for 1 ms to reduce CPU usage

        // Calculate the time elapsed since the last update
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        // Check if it's time to toggle the traffic light phase
        if (elapsedTime >= cycleDuration) {
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;

            // Send an update to the message queue
            
            _messageQueue.send(std::move(_currentPhase));
            // Reset the timer and randomize the cycle duration for the next cycle
            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = distribution(gen);
        }
    }
}

