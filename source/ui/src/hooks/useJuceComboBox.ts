import { useEffect, useState, useCallback } from 'react'
import { getComboBoxState } from '../lib/juce/index.js'

export function useJuceComboBox(id: string) {
  const [choiceIndex, setChoiceIndex] = useState(0)
  const [choices, setChoices] = useState<string[]>([])

  useEffect(() => {
    const state = getComboBoxState(id)
    setChoiceIndex(state.getChoiceIndex())
    setChoices([...state.properties.choices])

    const valueId = state.valueChangedEvent.addListener(() => setChoiceIndex(state.getChoiceIndex()))
    const propsId = state.propertiesChangedEvent.addListener(() => {
      setChoices([...state.properties.choices])
      setChoiceIndex(state.getChoiceIndex())
    })

    return () => {
      state.valueChangedEvent.removeListener(valueId)
      state.propertiesChangedEvent.removeListener(propsId)
    }
  }, [id])

  const setIndex = useCallback((i: number) => getComboBoxState(id).setChoiceIndex(i), [id])

  return { choiceIndex, choices, setChoiceIndex: setIndex }
}
