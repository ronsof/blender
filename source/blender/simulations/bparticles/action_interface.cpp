#include "action_interface.hpp"

namespace BParticles {

Action::~Action()
{
}

void ActionInterface::execute_action_for_subset(ArrayRef<uint> indices,
                                                std::unique_ptr<Action> &action)
{
  SmallVector<float> sub_current_times;
  SmallVector<float> sub_remaining_times;
  SmallVector<uint> particle_indices;
  for (uint i : indices) {
    particle_indices.append(m_particles.get_particle_index(i));
    sub_current_times.append(m_current_times[i]);
    sub_remaining_times.append(m_remaining_times[i]);
  }

  ParticleSet sub_particles(m_particles.block(), particle_indices);
  ActionInterface sub_interface(m_particle_allocator,
                                m_array_allocator,
                                sub_particles,
                                m_attribute_offsets,
                                sub_current_times,
                                sub_remaining_times,
                                m_event_info);
  action->execute(sub_interface);
}

ParticleFunctionCaller ParticleFunction::get_caller(AttributeArrays attributes,
                                                    EventInfo &event_info)
{
  ParticleFunctionCaller caller;
  caller.m_body = m_tuple_call;

  for (uint i = 0; i < m_function->input_amount(); i++) {
    StringRef input_name = m_function->input_name(i);
    void *ptr = nullptr;
    uint stride = 0;
    if (input_name.startswith("Event")) {
      StringRef event_attribute_name = input_name.drop_prefix("Event: ");
      ptr = event_info.get_info_array(event_attribute_name);
      stride = sizeof(float3); /* TODO make not hardcoded */
    }
    else if (input_name.startswith("Attribute")) {
      StringRef attribute_name = input_name.drop_prefix("Attribute: ");
      uint index = attributes.attribute_index(attribute_name);
      stride = attributes.attribute_stride(index);
      ptr = attributes.get_ptr(index);
    }
    else {
      BLI_assert(false);
    }

    BLI_assert(ptr);
    caller.m_attribute_buffers.append(ptr);
    caller.m_strides.append(stride);
  }

  return caller;
}

}  // namespace BParticles
