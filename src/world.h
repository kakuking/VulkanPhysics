#pragma once

#include "types.h"
#include "structs.h"

struct Particle {
    glm::vec3 center;
    float radius;
    float mass;
    glm::vec3 velocity;

    glm::vec3 accumulatedForce;
    glm::vec3 accumulatedAcceleration;

    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

    int level = 1;

    Particle() {}
    Particle(glm::vec3 newCenter, float newRadius){
        center = newCenter;
        radius = newRadius;
    }

    virtual void createVertices(){};
    virtual void updateLevel(int newLevel){};

    void translate(glm::vec3 t) {
        center += t;
        for(auto& vert: vertices){
            vert += t;
        }
    }

    void addForce(glm::vec3 force){
        accumulatedForce += force;
        accumulatedAcceleration += force/mass;
    }

    void resetForce(){
        accumulatedForce = {};
        accumulatedAcceleration = {};
    }
};

struct Circle: public Particle {
    Circle(glm::vec3 newCenter, float newRadius, float newMass){
        center = newCenter;
        radius = newRadius;
        mass = newMass;
    }

    Circle(glm::vec3 newCenter, float newRadius, float newMass, int newLevel){
        center = newCenter;
        radius = newRadius;
        mass = newMass;
        level = newLevel;
    }

    void createVertices(){
        float angle = glm::two_pi<float>() / (level + 2.f);

        float prevAngle = 0.0f;
        float curAngle = 0.0f;

        int centerIndex = vertices.size();
        vertices.push_back(center); // center is 0th
        for (size_t i = 0; i < level + 2; i++)
        {   
            curAngle = angle * (i + 1.f);

            glm::vec3 first = radius * glm::vec3(glm::cos(curAngle), glm::sin(curAngle), 0.f);
            glm::vec3 third = radius * glm::vec3(glm::cos(prevAngle), glm::sin(prevAngle), 0.f);

            prevAngle = curAngle;

            vertices.push_back(first + center);
            vertices.push_back(third + center);

            indices.push_back(vertices.size() - 2);
            indices.push_back(centerIndex);
            indices.push_back(vertices.size() - 1);
        }
    }
};

// 2D
struct BarnesHutNode{
    std::vector<Particle*> particles;
    glm::vec3 bottomLeftLimit, topRightLimit, center;


    BarnesHutNode* bottomLeft;
    BarnesHutNode* bottomRight;
    BarnesHutNode* topLeft;
    BarnesHutNode* topRight;

    void addParticle(Particle* newParticle){
        // first particle
        if(particles.size() < 1){
            particles.push_back(newParticle);

            bottomLeftLimit = {-1000.f, -1000.f, 0.f};
            topRightLimit = {1000.f, 1000.f, 0.f};


        }


    }
};

class World{
    std::vector<Particle> particles;
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    int level = 1;    

public:
    float G = 6.67e-2; // 10^7 times stronger than actually

    std::vector<Vertex> getVertices(){
        int currentOffset = 0;
        for (auto& p: particles){   
            for(size_t j = 0; j < p.vertices.size(); j++)
                vertices[currentOffset + j].position = p.vertices[j];
            
            currentOffset += p.vertices.size();
        }
        
        return vertices;
    }

    std::vector<uint32_t> getIndices(){
        return indices;
    }

    int getParticleCount(){
        return particles.size();
    }

    void setLevel(int newLevel){
        level = newLevel;
    }

    void addCircle(glm::vec3 center, float radius, float mass, glm::vec4 color){
        addCircle(center, radius, mass, color, level);
    }

    void addCircle(glm::vec3 center, float radius, float mass, glm::vec4 color, int level){
        Circle circle(center, radius, mass, level);
        circle.createVertices();

        int currentOffset = vertices.size();

        for(auto& vertex: circle.vertices){
            Vertex v;
            v.position = vertex;
            v.color = color;

            vertices.push_back(v);
        }

        for (auto& index: circle.indices)
        {
            indices.push_back(index + currentOffset);
        }

        particles.push_back(circle);
    }

    void update(float deltaTime){
        if(particles.size() < 2)
            return;

        for(auto& particle: particles){
            updatePosition(particle, deltaTime);
            particle.resetForce();
        }

        accumulateForces();
    }

private:
    void accumulateForces(){
        for (size_t i = 0; i < particles.size(); i++)
        {
            for (size_t j = i+1; j < particles.size(); j++)
            {
                glm::vec3 direction = particles[i].center - particles[j].center;    // Towards ith particle
                float distance = glm::length(direction);

                float magnitude = G * particles[i].mass * particles[j].mass / (distance * distance);

                particles[i].addForce(magnitude*-direction);
                particles[j].addForce(magnitude*direction);
            }
        }
    }

    void updatePosition(Particle& particle, float deltaTime) {
        glm::vec3 displacement = particle.velocity * deltaTime;
        glm::vec3 at = deltaTime * particle.accumulatedAcceleration;
        displacement += (float)0.5 * deltaTime * at;

        particle.translate(displacement);

        particle.velocity += at;
    }
};