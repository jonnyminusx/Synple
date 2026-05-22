import { useMidiLearnContext } from '../context/MidiLearnContext.js'

export function useMidiLearn(paramId: string) {
  const { state, startLearn, cancelLearn, clearLearn } = useMidiLearnContext()
  return {
    cc: paramId in state.assignments ? state.assignments[paramId] : null,
    isLearning: state.learningParam === paramId,
    startLearn: () => startLearn(paramId),
    cancelLearn,
    clearLearn: () => clearLearn(paramId),
  }
}
